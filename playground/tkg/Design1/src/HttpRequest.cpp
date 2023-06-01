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

HttpRequest::State HttpRequest::readRequest(HttpRequest &req, IReadCloser *rc) {
  std::string request;

  size_t size = rc->read(request, SOCKET_READ_SIZE);

  // todo(thara): move this logic to ConnectionSocket
  if (size == 0) {
    printf("closed fd = %d\n", req.sock_fd_);
    return HttpRequest::SocketClosed;
  }
  req.raw_data_ += request;

  std::cout << "read from socket(fd:" << req.sock_fd_ << ")"
            << ":'" << escape(req.raw_data_) << "'" << std::endl;
  if (req.state_ == ReadingStartLine && req.isActionable()) {
    req.trimToEndingChars();
    req.parseStartline();
    req.refresh();
    req.moveToNextState();
  }
  if (req.state_ == ReadingHeaders && req.isActionable()) {
    req.trimToEndingChars();
    req.parseHeaders();
    req.refresh();
    req.moveToNextState();
  }
  if (req.state_ == ReadingChunkedBody) {
    req.readChunkedBody();
  } else if (req.state_ == ReadingBody && req.isActionable()) {
    req.readBody();
  }

  return req.state_;
}

bool HttpRequest::isReceivingBody() {
  if (request_line_.method == GET || request_line_.method == DELETE || (!isChunked() && headers_.content_length == 0)) {
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
    throw NotAllowedException("Http Request: method not allowed");
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
    throw BadRequestException("Http Request: invalid request target");
  }
}

void HttpRequest::assignAndValidateVersion(const std::string &version) {
  if (version == "HTTP/1.1") {
    request_line_.version = HTTP1_1;
  } else {
    throw VersionNotSupportedException("Http Request: invalid version");
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
  // https://www.rfc-editor.org/rfc/rfc7230#section-5.4
  // validate required headers
  if (!hasField(HostField)) {
    throw BadRequestException("missing host header");
  }

  // https://www.rfc-editor.org/rfc/rfc7230#section-3.3.3
  // check if content-length and transfer-encoding are both present
  // todo: refactor hasField
  if (hasField(ContentLengthField) && headers_.transfer_encodings.size() != 0) {
    throw BadRequestException("both content-length and transfer-encoding are present");
  }
}

bool HttpRequest::hasField(HeaderField field) const { return received_fields_.find(field) != received_fields_.end(); }

void HttpRequest::insertIfNotDuplicate(HeaderField field, const char *error_msg) {
  if (received_fields_.find(field) != received_fields_.end()) {
    throw BadRequestException(error_msg);
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
      throw BadRequestException("Http Request: invalid port");
    }

    // todo: handle overflow
    headers_.host.port = std::atoi(port.c_str());
    if (headers_.host.port < MIN_PORT_NUM || headers_.host.port > MAX_PORT_NUM) {
      throw BadRequestException("Http Request: invalid port");
    }
  }
}

void HttpRequest::analyzeContentLength(const std::string &value) {
  // https://www.rfc-editor.org/rfc/rfc7230#section-3.3.3
  insertIfNotDuplicate(ContentLengthField, "Http Request: duplicated content length");

  if (!isAllDigit(value)) {
    throw BadRequestException("Http Request: invalid content-length");
  }

  // todo: handle overflow
  const int val = std::atoi(value.c_str());
  if (val < 0 || conf_.getMaxBodySize() < val) {
    throw BadRequestException("Http Request: invalid content-length");
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
      throw NotImplementedException("Http Request: unsupported transfer-encoding");
    } else {
      throw BadRequestException("Http Request: invalid transfer-encoding");
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
    throw BadRequestException("Http Request: invalid date");
  }

  // this checks if the date is valid
  std::time_t t = std::mktime(&headers_.date);
  if (t == -1) {
    throw BadRequestException("Http Request: invalid date");
  }
}

void HttpRequest::validateHeaderName(const std::string &name) {
  if (!isToken(name)) {
    throw BadRequestException("Http Request: invalid header name");
  }
}

void HttpRequest::validateHeaderValue(const std::string &value) {
  if (!isVchar(value)) {
    throw BadRequestException("Http Request: invalid header value");
  }
}

void HttpRequest::readChunkedBody() {
  while (true) {
    if (chunked_reading_state_ == ReadingChunkedSize) {
      std::string::size_type end = raw_data_.find(CRLF);
      if (end == std::string::npos) {
        return;
      }

      std::string hex = raw_data_.substr(0, end);
      std::stringstream ss(hex);
      ss >> std::hex >> chunked_size_;

      // todo(thara):there is unread data in socket
      if (chunked_size_ == 0) {
        rest_ = raw_data_.substr(end + 2);
        // todo(thara): I think we can remove this because I added re-initialization of HttpRequest
        chunked_reading_state_ = ReadingChunkedSize;
        moveToNextState();
        return;
      }

      raw_data_ = raw_data_.substr(end + 2);
      chunked_reading_state_ = ReadingChunkedData;
    }

    if (chunked_reading_state_ == ReadingChunkedData) {
      if (raw_data_.size() < (chunked_size_ + 2)) {
        return;
      }
      if (raw_data_.substr(chunked_size_, 2) != CRLF) {
        throw BadRequestException("Http Request: invalid chunked body");
      }

      body_ += raw_data_.substr(0, chunked_size_);
      raw_data_ = raw_data_.substr(chunked_size_ + 2);

      chunked_reading_state_ = ReadingChunkedSize;
    }
  }
  return;
}

void HttpRequest::readBody() {
  body_ = raw_data_;
  moveToNextState();
}

bool HttpRequest::isActionable() {
  if (state_ == ReadingStartLine || state_ == ReadingHeaders) {
    return raw_data_.find(getEndingChars()) != std::string::npos;
  } else if (state_ == ReadingBody) {
    // this accepts body that is larger than content-length
    return raw_data_.size() >= headers_.content_length;
  } else {
    throw std::runtime_error("invalid HttpRequest state");
  }
}

// isActionable must be called before calling this function
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
      if (isReceivingBody()) {
        if (isChunked()) {
          state_ = ReadingChunkedBody;
        } else {
          state_ = ReadingBody;
        }
      } else {
        state_ = FinishedReading;
      }
      break;
    case ReadingBody:
    case ReadingChunkedBody:
      state_ = FinishedReading;
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
