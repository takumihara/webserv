#include "HttpRequestReader.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "Config/validation.h"
#include "HttpException.hpp"
#include "const.hpp"
#include "debug.hpp"
#include "helper.hpp"

#define MIN_PORT_NUM 0
#define MAX_PORT_NUM 65535

HttpRequestReader::State HttpRequestReader::read() {
  std::string buff;

  size_t size = rc_->read(buff, SOCKET_READ_SIZE);

  if (size == 0) {
    DEBUG_PRINTF("closed fd = %d\n", sock_fd_);
    throw std::runtime_error("closed client socket");
  }
  raw_data_ += buff;

  std::cout << "read from socket(fd:" << sock_fd_ << ")"
            << ":'" << escape(raw_data_) << "'" << std::endl;
  if (state_ == ReadingStartLine && isActionable()) {
    trimToEndingChars();
    parseStartline();
    moveToNextState();
  }
  if (state_ == ReadingHeaders && isActionable()) {
    trimToEndingChars();
    parseHeaders();
    moveToNextState();
  }
  if (state_ == ReadingChunkedBody) {
    readChunkedBody();
  } else if (state_ == ReadingBody && isActionable()) {
    readBody();
  }

  return state_;
}

bool HttpRequestReader::isReceivingBody() {
  std::cout << request_.headers_.content_length << std::endl;
  if ((!request_.isChunked() && request_.headers_.content_length == 0)) {
    return false;
  }
  return true;
}
void HttpRequestReader::parseStartline() {
  std::stringstream ss = std::stringstream(raw_data_);
  std::string method;
  std::string request_target;
  std::string version;

  std::getline(ss, method, SP);
  std::getline(ss, request_target, SP);
  std::getline(ss, version, '\r');
  // ss >> version;

  assignAndValidateMethod(method);
  assignAndValidateRequestTarget(request_target);
  assignAndValidateVersion(version);

  DEBUG_PUTS("REQUEST LINE PARSED");
  DEBUG_PRINTF("method: '%s' request target: absolute_path -'%s' version: '%s'\n", method.c_str(),
               request_target.c_str(), version.c_str());
}

void HttpRequestReader::assignAndValidateMethod(const std::string &method) {
  if (method == "GET") {
    request_.method_ = HttpRequest::GET;
  } else if (method == "POST") {
    request_.method_ = HttpRequest::POST;
  } else if (method == "DELETE") {
    request_.method_ = HttpRequest::DELETE;
  } else if (method == "PUT" || method == "PATCH" || method == "HEAD" || method == "OPTIONS") {
    throw MethodNotAllowedException("Http Request: method not allowed");
  } else {
    throw NotImplementedException("Http Request: invalid method: '" + method + "'");
  }
}

void HttpRequestReader::assignAndValidateRequestTarget(const std::string &request_target) {
  if (request_target[0] == '/') {
    try {
      request_.request_target_ = URI::parseRequestURI(request_target);
    } catch (const std::runtime_error &e) {
      DEBUG_PUTS(e.what());
      throw BadRequestException("Http Request: invalid request target");
    }
  } else {
    throw BadRequestException("Http Request: invalid request target");
  }
}

void HttpRequestReader::assignAndValidateVersion(const std::string &version) {
  if (version == "HTTP/1.1") {
    request_.version_ = HttpRequest::HTTP1_1;
  } else {
    throw VersionNotSupportedException("Http Request: invalid version");
  }
}

void HttpRequestReader::initAnalyzeFuncs(
    std::map<std::string, void (HttpRequestReader::*)(const std::string &)> &analyze_funcs) {
  analyze_funcs["host"] = &HttpRequestReader::analyzeHost;
  analyze_funcs["content-length"] = &HttpRequestReader::analyzeContentLength;
  analyze_funcs["transfer-encoding"] = &HttpRequestReader::analyzeTransferEncoding;
  analyze_funcs["date"] = &HttpRequestReader::analyzeDate;
  analyze_funcs["content-type"] = &HttpRequestReader::analyzeContentType;
}

void HttpRequestReader::parseHeaders() {
  std::stringstream ss(raw_data_);

  std::string::size_type start = 0;
  std::string::size_type end = raw_data_.find(CRLF, start);

  std::map<std::string, void (HttpRequestReader::*)(const std::string &)> analyze_funcs;
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
  request_.printHeaders();
}

void HttpRequestReader::validateHeaders() {
  // https://www.rfc-editor.org/rfc/rfc7230#section-5.4
  // validate required headers
  if (!hasField(HostField)) {
    throw BadRequestException("missing host header");
  }

  // https://www.rfc-editor.org/rfc/rfc7230#section-3.3.3
  // check if content-length and transfer-encoding are both present
  // todo: refactor hasField
  if (hasField(ContentLengthField) && request_.headers_.transfer_encodings.size() != 0) {
    throw BadRequestException("both content-length and transfer-encoding are present");
  }
}

bool HttpRequestReader::hasField(HeaderField field) const {
  return received_fields_.find(field) != received_fields_.end();
}

void HttpRequestReader::insertIfNotDuplicate(HeaderField field, const char *error_msg) {
  if (received_fields_.find(field) != received_fields_.end()) {
    throw BadRequestException(error_msg);
  }
  received_fields_.insert(field);
}

void HttpRequestReader::analyzeHost(const std::string &value) {
  std::string hostHeader = trimUntilCRLF(value);

  insertIfNotDuplicate(HostField, "Http Request: duplicated host header");

  // parse host
  std::string port;
  std::size_t colonPos = hostHeader.find(':');
  if (colonPos != std::string::npos) {
    // If there is a colon in the string, split it into hostname and port
    request_.headers_.host.uri_host = hostHeader.substr(0, colonPos);
    port = hostHeader.substr(colonPos + 1);
  } else {
    // If there is no colon, the whole string is the hostname
    request_.headers_.host.uri_host = hostHeader;
    request_.headers_.host.port = DEFAULT_HTTP_PORT;
  }

  // validate uri-host

  // validate port
  if (!port.empty()) {
    if (!isAllDigit(port)) {
      throw BadRequestException("Http Request: invalid port");
    }

    // todo: handle overflow
    request_.headers_.host.port = std::atoi(port.c_str());
    if (request_.headers_.host.port < MIN_PORT_NUM || request_.headers_.host.port > MAX_PORT_NUM) {
      throw BadRequestException("Http Request: invalid port");
    }
  }
}

void HttpRequestReader::analyzeContentLength(const std::string &value) {
  // https://www.rfc-editor.org/rfc/rfc7230#section-3.3.3
  insertIfNotDuplicate(ContentLengthField, "Http Request: duplicated content length");

  if (!isAllDigit(value)) {
    throw BadRequestException("Http Request: invalid content-length");
  }

  // todo: handle overflow
  const int val = std::atoi(value.c_str());
  if (val < 0 || conf_->getMaxBodySize() < val) {
    throw BadRequestException("Http Request: invalid content-length");
  }
  request_.headers_.content_length = val;
}

void HttpRequestReader::analyzeTransferEncoding(const std::string &value) {
  std::stringstream ss(value);
  std::string encoding;
  while (std::getline(ss, encoding, ',')) {
    toLower(encoding);
    encoding = trimOws(encoding);
    if (encoding == "chunked") {
      request_.headers_.transfer_encodings.push_back(HttpRequest::Chunked);
    } else if (encoding == "gzip" || encoding == "compress" || encoding == "deflate") {
      throw NotImplementedException("Http Request: unsupported transfer-encoding");
    } else {
      throw BadRequestException("Http Request: invalid transfer-encoding");
    }
  }
}

void HttpRequestReader::analyzeDate(const std::string &value) {
  std::string dateStr = trimUntilCRLF(value);
  insertIfNotDuplicate(DateField, "Http Request: duplicated date");

  std::istringstream ss(dateStr);

  // this checks if the format matches
  ss >> std::get_time(&request_.headers_.date, "%a, %d %b %Y %H:%M:%S");
  if (ss.fail()) {
    std::cout << "ss.fail()" << std::endl;
    throw BadRequestException("Http Request: invalid date");
  }

  // this checks if the date is valid
  std::time_t t = std::mktime(&request_.headers_.date);
  if (t == -1) {
    throw BadRequestException("Http Request: invalid date");
  }
}

void HttpRequestReader::analyzeContentType(const std::string &value) {
  insertIfNotDuplicate(ContentTypeField, "Http Request: duplicated content type");

  request_.headers_.content_type = value;
}

void HttpRequestReader::validateHeaderName(const std::string &name) {
  if (!isToken(name)) {
    throw BadRequestException("Http Request: invalid header name");
  }
}

void HttpRequestReader::validateHeaderValue(const std::string &value) {
  if (!isVchar(value)) {
    throw BadRequestException("Http Request: invalid header value");
  }
}

void HttpRequestReader::readChunkedBody() {
  while (true) {
    if (chunked_reading_state_ == ReadingChunkedSize) {
      std::string::size_type end = raw_data_.find(CRLF);
      if (end == std::string::npos) {
        return;
      }

      std::string hex = raw_data_.substr(0, end);
      std::stringstream ss(hex);
      ss >> std::hex >> chunked_size_;

      if (chunked_size_ == 0) {
        rest_ = raw_data_.substr(end + 2);
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

      request_.body_ += raw_data_.substr(0, chunked_size_);
      raw_data_ = raw_data_.substr(chunked_size_ + 2);

      chunked_reading_state_ = ReadingChunkedSize;
    }
  }
  return;
}

void HttpRequestReader::readBody() {
  request_.body_ = raw_data_.substr(0, request_.headers_.content_length);
  moveToNextState();
}

bool HttpRequestReader::isActionable() {
  if (state_ == ReadingStartLine || state_ == ReadingHeaders) {
    return raw_data_.find(getEndingChars()) != std::string::npos;
  } else if (state_ == ReadingBody) {
    // this accepts body that is larger than content-length
    return raw_data_.size() >= request_.headers_.content_length;
  } else {
    throw std::runtime_error("invalid HttpRequestReader state");
  }
}

// isActionable must be called before calling this function
void HttpRequestReader::trimToEndingChars() {
  const std::string ending_chars = getEndingChars();
  size_t end_pos = raw_data_.find(ending_chars);

  rest_ = raw_data_.substr(end_pos + ending_chars.size());
  raw_data_ = raw_data_.substr(0, end_pos + ending_chars.size());
}

std::string HttpRequestReader::getEndingChars() const {
  switch (state_) {
    case ReadingStartLine:
      return CRLF;
    case ReadingHeaders:
      return std::string(CRLF) + CRLF;
    default:
      throw std::runtime_error("invalid HttpRequestReader state");
  }
}

void HttpRequestReader::moveToNextState() {
  raw_data_ = rest_;
  rest_.clear();
  switch (state_) {
    case ReadingStartLine:
      state_ = ReadingHeaders;
      break;
    case ReadingHeaders:
      if (isReceivingBody()) {
        if (request_.isChunked()) {
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
      throw std::runtime_error("invalid HttpRequestReader state");
  }
}
