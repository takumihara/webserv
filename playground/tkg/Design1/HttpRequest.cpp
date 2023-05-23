#include "HttpRequest.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>

#include "Config/validation.h"
#include "EventManager.hpp"
#include "const.hpp"
#include "helper.hpp"

bool HttpRequest::readRequest(EventManager &event_manager) {
  char request[SOCKET_READ_SIZE + 1];
  bzero(request, SOCKET_READ_SIZE + 1);
  std::string req_str;
  int size = read(sock_fd_, request, SOCKET_READ_SIZE);
  bool finishedReading = false;

  if (size == -1) {
    // todo: no exception
    throw std::runtime_error("read error");
  } else if (size == 0) {
    printf("closed fd = %d\n", sock_fd_);
    close(sock_fd_);
    event_manager.removeConnectionSocket(sock_fd_);
  } else {
    req_str = std::string(request);
    raw_data_ += req_str;

    std::cout << "read from socket(fd:" << sock_fd_ << ")"
              << ":'" << escape(raw_data_) << "'" << std::endl;

    if (state_ == ReadingStartLine && isActionable()) {
      trimToEndingChars();
      parseStartline();
      refresh();
      moveToNextState();
    }
    if (state_ == ReadingHeaders && isActionable()) {
      trimToEndingChars();
      parseHeaders();
      refresh();
      if (!isReceivingBody()) {
        finishedReading = true;
      } else {
        moveToNextState();
      }
    }
    if (state_ == ReadingChunkedBody) {
      finishedReading = readChunkedBody();
    } else if (state_ == ReadingBody && isActionable()) {
      body_ = raw_data_;
      finishedReading = true;
    }

    if (finishedReading) {
      DEBUG_PRINTF("FINISHED READING: %s \n", escape(body_).c_str());
      event_manager.addChangedEvents((struct kevent){sock_fd_, EVFILT_READ, EV_DISABLE, 0, 0, 0});
      event_manager.addChangedEvents((struct kevent){sock_fd_, EVFILT_TIMER, EV_DISABLE, 0, 0, NULL});
    }
  }

  return finishedReading;
}

bool HttpRequest::isReceivingBody() {
  // todo: check content length
  DEBUG_PUTS("receiving body");
  return request_line_.method != GET;
}

void HttpRequest::parseStartline() {
  std::stringstream ss = std::stringstream(raw_data_);
  std::string method;
  std::string requestTarget;
  std::string version;

  std::getline(ss, method, SP);
  std::getline(ss, requestTarget, SP);
  std::getline(ss, version, '\r');
  // ss >> version;

  assignAndValidateMethod(method);
  assignAndValidateRequestTarget(requestTarget);
  assignAndValidateVersion(version);

  DEBUG_PUTS("REQUEST LINE PARSED");
  DEBUG_PRINTF("method: '%s' request target: absolute_path -'%s' query - '%s' version: '%s'\n", method.c_str(),
               request_line_.requestTarget.absolutePath.c_str(), request_line_.requestTarget.query.c_str(),
               version.c_str());
}

void HttpRequest::assignAndValidateMethod(const std::string &method) {
  if (method == "GET") {
    request_line_.method = GET;
  } else if (method == "POST") {
    request_line_.method = POST;
  } else {
    throw std::runtime_error("Http Request: invalid or unsupported method");
  }
}

void HttpRequest::assignAndValidateRequestTarget(const std::string &requestTarget) {
  if (requestTarget[0] == '/') {
    // todo(thara): more validation
    request_line_.requestTarget.type = OriginForm;
    size_t pos = requestTarget.find_first_of('?');
    request_line_.requestTarget.absolutePath = requestTarget.substr(0, pos);
    if (pos != std::string::npos) {
      request_line_.requestTarget.query = requestTarget.substr(requestTarget.find_first_of('?') + 1);
    }
  } else {
    throw std::runtime_error("Http Request: invalid or unsupported request target");
  }
}

void HttpRequest::assignAndValidateVersion(const std::string &version) {
  if (version == "HTTP/1.1") {
    request_line_.version = HTTP1_1;
  } else {
    throw std::runtime_error("Http Request: invalid or unsupported version");
  }
}

void initAnalyzeFuncs(std::map<std::string, void (HttpRequest::*)(const std::string &)> &analyze_funcs) {
  analyze_funcs["host"] = &HttpRequest::analyzeHost;
  analyze_funcs["content-length"] = &HttpRequest::analyzeContentLength;
  analyze_funcs["transfer-encoding"] = &HttpRequest::analyzeTransferEncoding;
  analyze_funcs["date"] = &HttpRequest::analyzeDate;
}

void HttpRequest::parseHeaders() {
  std::stringstream ss(raw_data_);

  std::string::size_type start = 0;
  std::string::size_type end = raw_data_.find("\r\n", start);

  std::map<std::string, void (HttpRequest::*)(const std::string &)> analyze_funcs;
  initAnalyzeFuncs(analyze_funcs);

  while (end != std::string::npos) {
    std::string line = raw_data_.substr(start, end - start);
    std::stringstream lineStream(line);
    std::string name, value;

    if (std::getline(lineStream, name, ':')) {
      if (std::getline(lineStream, value)) {
        size_t value_start = value.find_first_not_of(" \t");
        if (value_start != std::string::npos) {
          value = value.substr(value_start);
        } else {
          value = "";
        }

        toLower(name);

        validateHeaderName(name);
        if (analyze_funcs.find(name) != analyze_funcs.end()) {
          (this->*analyze_funcs[name])(value);
        }
      }
    }

    start = end + 2;
    end = raw_data_.find("\r\n", start);
  }

  DEBUG_PUTS("HEADER PARSED");
  DEBUG_PRINTF("host: %s \n", (headers_.host.uri_host + ":" + headers_.host.port).c_str());
  DEBUG_PRINTF("content-length: %zu \n", headers_.content_length);
  DEBUG_PRINTF("transfer-encoding: %s \n", isChunked() ? "chunked" : "none");
  char buf[30];
  std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S", &headers_.date);
  DEBUG_PRINTF("date: %s \n", buf);
}

void HttpRequest::analyzeHost(const std::string &value) {
  std::string hostHeader = value.substr(0, value.find(CRLF));

  // todo: check if there is no duplication
  std::regex hostPattern(
      "^(([a-zA-Z0-9-]+\\.)*[a-zA-Z0-9-]+|([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}))(\\:[0-9]{1,5})?$");
  if (!std::regex_match(hostHeader, hostPattern)) {
    throw std::runtime_error("Http Request: invalid or unsupported host header");
  }

  std::size_t colonPos = hostHeader.find(':');

  if (colonPos != std::string::npos) {
    // If there is a colon in the string, split it into hostname and port
    headers_.host.uri_host = hostHeader.substr(0, colonPos);
    headers_.host.port = hostHeader.substr(colonPos + 1);
  } else {
    // If there is no colon, the whole string is the hostname
    headers_.host.uri_host = hostHeader;
  }
}

void HttpRequest::analyzeContentLength(const std::string &value) {
  // todo: check if there is no duplication
  if (!isAllDigit(value)) {
    throw std::runtime_error("Http Request: invalid content-length");
  }
  headers_.content_length = std::atoi(value.c_str());
  // todo: check if content length is too big
}

void HttpRequest::analyzeTransferEncoding(const std::string &value) {
  std::stringstream ss(value);
  std::string encoding;
  while (std::getline(ss, encoding, ',')) {
    toLower(encoding);
    if (encoding == "chunked") {
      headers_.transferEncodings.push_back(Chunked);
    } else {
      throw std::runtime_error("Http Request: invalid or unsupported transfer-encoding");
    }
  }
}

void HttpRequest::analyzeDate(const std::string &value) {
  std::string dateStr = value.substr(0, value.find(CRLF));

  std::istringstream ss(dateStr);

  ss >> std::get_time(&headers_.date, "%a, %d %b %Y %H:%M:%S");
  if (ss.fail()) {
    std::cout << "ss.fail()" << std::endl;
    throw std::runtime_error("Http Request: invalid date");
  }

  std::time_t t = std::mktime(&headers_.date);
  if (t == -1) {
    std::cout << "t == -1" << std::endl;
    throw std::runtime_error("Http Request: invalid date");
  }

  // this validation might be too strict
  // char buf[30];
  // std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S", &headers_.date);

  // if (dateStr != std::string(buf) + " GMT") {
  //   std::cout << "dateStr: '" << dateStr << "'" << std::endl;
  //   std::cout << "buf: '" << buf << "'" << std::endl;
  //   throw std::runtime_error("Http Request: invalid date");
  // }
}

void HttpRequest::validateHeaderName(const std::string &name) {
  if (name == "" || *name.rbegin() == SP) {
    throw std::runtime_error("Http Request: invalid header name");
  }
}

void HttpRequest::validateHeaderValue(const std::string &value) {
  (void)value;
  // todo
}

bool HttpRequest::readChunkedBody() {
  while (true) {
    if (chunked_reading_state_ == ReadingChunkedSize) {
      std::string::size_type end = raw_data_.find(CRLF);
      if (end == std::string::npos) {
        return false;
      }

      std::string hex = raw_data_.substr(0, end);
      std::stringstream ss(hex);
      ss >> std::hex >> chunked_size_;

      if (chunked_size_ == 0) {
        return true;
      }

      raw_data_ = raw_data_.substr(end + 2);
      chunked_reading_state_ = ReadingChunkedData;
    }

    if (chunked_reading_state_ == ReadingChunkedData) {
      if (raw_data_.size() < (chunked_size_ + 2)) {
        return false;
      }
      if (raw_data_.substr(chunked_size_, 2) != CRLF) {
        throw std::runtime_error("Http Request: invalid chunked body");
      }

      body_ += raw_data_.substr(0, chunked_size_);
      raw_data_ = raw_data_.substr(chunked_size_ + 2);

      chunked_reading_state_ = ReadingChunkedSize;
    }
  }
  return false;
}

void HttpRequest::readBody() { body_ = raw_data_; }

bool HttpRequest::isActionable() {
  if (state_ == ReadingStartLine || state_ == ReadingHeaders || state_ == ReadingBody) {
    return raw_data_.find(getEndingChars()) != std::string::npos;
  } else {
    throw std::runtime_error("invalid HttpRequest state");
  }
}

void HttpRequest::trimToEndingChars() {
  const std::string ending_chars = getEndingChars();
  size_t end_pos = raw_data_.find(ending_chars);

  rest_ = raw_data_.substr(end_pos + ending_chars.size());
  raw_data_ = raw_data_.substr(0, end_pos + ending_chars.size());
}

std::string HttpRequest::getEndingChars() const {
  switch (state_) {
    case ReadingStartLine:
      return CRLF;
    case ReadingHeaders:
      return std::string(CRLF) + CRLF;
    case ReadingBody:
      return CRLF;
    default:
      throw std::runtime_error("invalid HttpRequest state");
  }
}

void HttpRequest::moveToNextState() {
  switch (state_) {
    case ReadingStartLine:
      state_ = ReadingHeaders;
      break;
    case ReadingHeaders:
      if (isChunked()) {
        state_ = ReadingChunkedBody;
      } else {
        state_ = ReadingBody;
      }
      break;
    default:
      throw std::runtime_error("invalid HttpRequest state");
  }
}

void HttpRequest::refresh() {
  raw_data_ = rest_;
  rest_ = "";
}

const std::string &HttpRequest::getBody() const { return body_; }

const HttpRequest::RequestTarget &HttpRequest::getRequestTarget() const { return request_line_.requestTarget; }
const HttpRequest::Host &HttpRequest::getHost() const { return headers_.host; }
bool HttpRequest::isChunked() {
  std::vector<TransferEncoding> &transferEncodings = headers_.transferEncodings;
  return std::find(transferEncodings.begin(), transferEncodings.end(), HttpRequest::Chunked) != transferEncodings.end();
}
