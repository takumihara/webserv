#include "HttpRequest.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "Config/validation.h"
#include "EventManager.hpp"
#include "const.hpp"
#include "helper.hpp"

#define MIN_PORT_NUM 0
#define MAX_PORT_NUM 65535

bool HttpRequest::readRequest(EventManager &event_manager) {
  char request[SOCKET_READ_SIZE + 1];
  bzero(request, SOCKET_READ_SIZE + 1);
  std::string req_str;
  int size = read(sock_fd_, request, SOCKET_READ_SIZE);
  bool finishedReading = false;

  if (size == -1) {
    throw std::runtime_error("read error");
  } else if (size == 0) {
    printf("closed fd = %d\n", sock_fd_);
    // closeとEVFILT_TIMERのDELETEはワンセット
    close(sock_fd_);
    event_manager.addChangedEvents((struct kevent){sock_fd_, EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
    event_manager.remove(std::pair<t_id, t_type>(sock_fd_, FD));
  } else {
    req_str = std::string(request);
    raw_data_ += req_str;

    std::cout << "read from socket(fd:" << sock_fd_ << ")"
              << ":'" << escape(raw_data_) << "'" << std::endl;
    std::cout << "state: " << state_ << std::endl;
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
      moveToNextState();
      // event_manager.addChangedEvents((struct kevent){sock_fd_, EVFILT_READ, EV_DISABLE, 0, 0, 0});
      // event_manager.addChangedEvents((struct kevent){sock_fd_, EVFILT_TIMER, EV_DISABLE, 0, 0, NULL});
    }
  }

  return finishedReading;
}

bool HttpRequest::isReceivingBody() {
  if (request_line_.method == GET || request_line_.method == DELETE || !isChunked() || headers_.content_length == 0) {
    return false;
  }
  return true;
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
               request_line_.request_target.absolute_path.c_str(), request_line_.request_target.query.c_str(),
               version.c_str());
}

void HttpRequest::assignAndValidateMethod(const std::string &method) {
  if (method == "GET") {
    request_line_.method = GET;
  } else if (method == "POST") {
    request_line_.method = POST;
  } else if (method == "DELETE") {
    request_line_.method = DELETE;
  } else if (method == "PUT" || method == "PATCH" || method == "HEAD" || method == "OPTIONS") {
    throw NotAllowedException();
  } else {
    throw std::runtime_error("Http Request: invalid method");
  }
}

void HttpRequest::assignAndValidateRequestTarget(const std::string &request_target) {
  if (request_target[0] == '/') {
    // todo(thara): more validation
    request_line_.request_target.type = OriginForm;
    size_t pos = request_target.find_first_of('?');
    request_line_.request_target.absolute_path = request_target.substr(0, pos);
    if (pos != std::string::npos) {
      request_line_.request_target.query = request_target.substr(request_target.find_first_of('?') + 1);
    }
  } else {
    DEBUG_PUTS("Http Request: invalid request target");
    throw BadRequestException();
  }
}

void HttpRequest::assignAndValidateVersion(const std::string &version) {
  if (version == "HTTP/1.1") {
    request_line_.version = HTTP1_1;
  } else {
    throw VersionNotSupportedException();
  }
}

void HttpRequest::initAnalyzeFuncs(std::map<std::string, void (HttpRequest::*)(const std::string &)> &analyze_funcs) {
  analyze_funcs["host"] = &HttpRequest::analyzeHost;
  analyze_funcs["content-length"] = &HttpRequest::analyzeContentLength;
  analyze_funcs["transfer-encoding"] = &HttpRequest::analyzeTransferEncoding;
  analyze_funcs["date"] = &HttpRequest::analyzeDate;
}

void HttpRequest::parseHeaders() {
  std::stringstream ss(raw_data_);

  std::string::size_type start = 0;
  std::string::size_type end = raw_data_.find(CRLF, start);

  std::map<std::string, void (HttpRequest::*)(const std::string &)> analyze_funcs;
  initAnalyzeFuncs(analyze_funcs);
  while (true) {
    std::string line = raw_data_.substr(start, end - start);
    // this is the end of headers
    if (line.empty()) {
      break;
    }

    std::stringstream lineStream(line);
    std::string name, value;

    std::getline(lineStream, name, ':');
    std::getline(lineStream, value);
    value = trimOws(value);
    toLower(name);

    validateHeaderName(name);
    validateHeaderValue(value);
    if (analyze_funcs.find(name) != analyze_funcs.end()) {
      (this->*analyze_funcs[name])(value);
    }

    start = end + 2;
    end = raw_data_.find(CRLF, start);
  }

  validateHeaders();
  printHeaders();
}

void HttpRequest::validateHeaders() {
  if (received_fields_.find(HostField) == received_fields_.end()) {
    DEBUG_PUTS("Http Request: missing host header");
    throw BadRequestException();
  }
}

void HttpRequest::insertIfNotDuplicate(HeaderField field, const char *error_msg) {
  if (received_fields_.find(field) != received_fields_.end()) {
    (void)error_msg;
    DEBUG_PUTS(error_msg);
    throw BadRequestException();
  }
  received_fields_.insert(field);
}

void HttpRequest::analyzeHost(const std::string &value) {
  std::string hostHeader = trimUntilCRLF(value);

  insertIfNotDuplicate(HostField, "Http Request: duplicated host header");

  // parse host
  std::string port;
  std::size_t colonPos = hostHeader.find(':');
  if (colonPos != std::string::npos) {
    // If there is a colon in the string, split it into hostname and port
    headers_.host.uri_host = hostHeader.substr(0, colonPos);
    port = hostHeader.substr(colonPos + 1);
  } else {
    // If there is no colon, the whole string is the hostname
    headers_.host.uri_host = hostHeader;
    headers_.host.port = DEFAULT_HTTP_PORT;
  }

  // validate uri-host

  // validate port
  if (!port.empty()) {
    if (!isAllDigit(port)) {
      DEBUG_PUTS("Http Request: invalid port");
      throw BadRequestException();
    }

    // todo: handle overflow
    headers_.host.port = std::atoi(port.c_str());
    if (headers_.host.port < MIN_PORT_NUM || headers_.host.port > MAX_PORT_NUM) {
      DEBUG_PUTS("Http Request: invalid port");
      throw BadRequestException();
    }
  }
}

void HttpRequest::analyzeContentLength(const std::string &value) {
  insertIfNotDuplicate(ContentLengthField, "Http Request: duplicated content length");

  if (!isAllDigit(value)) {
    DEBUG_PUTS("Http Request: invalid content-length");
    throw BadRequestException();
  }

  const int val = std::atoi(value.c_str());
  // todo: check if content length is too big
  if (val < 0) {
    DEBUG_PUTS("Http Request: invalid content-length");
    throw BadRequestException();
  }
  headers_.content_length = val;
}

void HttpRequest::analyzeTransferEncoding(const std::string &value) {
  std::stringstream ss(value);
  std::string encoding;
  while (std::getline(ss, encoding, ',')) {
    toLower(encoding);
    encoding = trimOws(encoding);
    if (encoding == "chunked") {
      headers_.transfer_encodings.push_back(Chunked);
    } else if (encoding == "gzip" || encoding == "compress" || encoding == "deflate") {
      DEBUG_PUTS("Http Request: unsupported transfer-encoding");
      throw NotImplementedException();
    } else {
      DEBUG_PUTS("Http Request: invalid transfer-encoding");
      throw BadRequestException();
    }
  }
}

void HttpRequest::analyzeDate(const std::string &value) {
  std::string dateStr = trimUntilCRLF(value);
  insertIfNotDuplicate(DateField, "Http Request: duplicated date");

  std::istringstream ss(dateStr);

  // this checks if the format matches
  ss >> std::get_time(&headers_.date, "%a, %d %b %Y %H:%M:%S");
  if (ss.fail()) {
    std::cout << "ss.fail()" << std::endl;
    DEBUG_PUTS("Http Request: invalid date");
    throw BadRequestException();
  }

  // this checks if the date is valid
  std::time_t t = std::mktime(&headers_.date);
  if (t == -1) {
    DEBUG_PUTS("Http Request: invalid date");
    throw BadRequestException();
  }
}

void HttpRequest::validateHeaderName(const std::string &name) {
  if (!isToken(name)) {
    DEBUG_PUTS("Http Request: invalid header name");
    throw BadRequestException();
  }
}

void HttpRequest::validateHeaderValue(const std::string &value) {
  // todo(thara): check for obs-fold and return 400
  if (!isVchar(value)) {
    DEBUG_PUTS("Http Request: invalid header value");
    throw BadRequestException();
  }
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

      // todo(thara):there is unread data in socket
      if (chunked_size_ == 0) {
        rest_ = raw_data_.substr(end + 2);
        // todo(thara): I think we can remove this because I added re-initialization of HttpRequest
        chunked_reading_state_ = ReadingChunkedSize;
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
        DEBUG_PUTS("Http Request: invalid chunked body");
        throw BadRequestException();
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
    case ReadingBody:
    case ReadingChunkedBody:
      // todo(thara): I think we can remove this because I added re-initialization of HttpRequest
      state_ = ReadingStartLine;
      break;
    default:
      throw std::runtime_error("invalid HttpRequest state");
  }
}

void HttpRequest::refresh() {
  raw_data_ = rest_;
  rest_ = "";
}
bool HttpRequest::methodIs(Method method) const { return request_line_.method == method; };

const std::string &HttpRequest::getBody() const { return body_; }

const HttpRequest::RequestTarget &HttpRequest::getRequestTarget() const { return request_line_.request_target; }
const HttpRequest::Host &HttpRequest::getHost() const { return headers_.host; }
bool HttpRequest::isChunked() {
  std::vector<TransferEncoding> &transferEncodings = headers_.transfer_encodings;
  return std::find(transferEncodings.begin(), transferEncodings.end(), HttpRequest::Chunked) != transferEncodings.end();
}

void HttpRequest::printHeaders() {
  DEBUG_PUTS("HEADER PARSED");
  DEBUG_PRINTF("host: %s \n", (headers_.host.uri_host + ":" + std::to_string(headers_.host.port)).c_str());
  DEBUG_PRINTF("content-length: %zu \n", headers_.content_length);
  DEBUG_PRINTF("transfer-encoding: %s \n", isChunked() ? "chunked" : "none");
  char buf[30];
  std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S", &headers_.date);
  DEBUG_PRINTF("date: %s \n", buf);
}
