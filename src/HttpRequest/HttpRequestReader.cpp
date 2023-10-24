#include "HttpRequestReader.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>

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
  raw_data_.insert(raw_data_.end(), buff.begin(), buff.end());

  DEBUG_PRINTF("read from socket(fd:%d):'%s'\n", sock_fd_,
               escape(std::string(raw_data_.begin(), raw_data_.end())).c_str());
  if (state_ == ReadingStartLine && isActionable()) {
    trimToEndingChars();
    parseStartline();
    moveToNextState();
  }
  if (state_ == ReadingHeaders && isActionable()) {
    trimToEndingChars();
    if (parseHeaders()) {
      moveToNextState();
    } else {
      raw_data_ = rest_;
      rest_.clear();
    }
  }
  if (state_ == ReadingChunkedBody) {
    readChunkedBody();
  } else if (state_ == ReadingBody && isActionable()) {
    readBody();
  }
  return state_;
}

bool HttpRequestReader::isReceivingBody() {
  if ((!request_.isChunked() && request_.headers_.content_length == 0)) {
    return false;
  }
  return true;
}
void HttpRequestReader::parseStartline() {
  std::stringstream ss(std::string(raw_data_.begin(), raw_data_.end()));
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
  analyze_funcs["cookie"] = &HttpRequestReader::analyzeCookie;
}

// parseHeaders returns true if all headers are parsed
bool HttpRequestReader::parseHeaders() {
  std::stringstream ss(std::string(raw_data_.begin(), raw_data_.end()));

  std::vector<char>::iterator start = raw_data_.begin();
  std::vector<char>::iterator end = std::search(raw_data_.begin(), raw_data_.end(), CRLFVec().begin(), CRLFVec().end());

  std::map<std::string, void (HttpRequestReader::*)(const std::string &)> analyze_funcs;
  initAnalyzeFuncs(analyze_funcs);

  while (end != raw_data_.end()) {
    std::vector<char> line(start, end);
    // this is the end of headers
    if (line.empty()) {
      validateHeaders();
      request_.printHeaders();
      return true;
    }

    std::stringstream lineStream(std::string(line.begin(), line.end()));
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
    end = std::search(start, raw_data_.end(), CRLFVec().begin(), CRLFVec().end());
  }

  return false;
}

void HttpRequestReader::validateHeaders() {
  // https://www.rfc-editor.org/rfc/rfc7230#section-5.4
  // validate required headers
  if (!hasField(HostField)) {
    throw BadRequestException("missing host header");
  }

  // https://www.rfc-editor.org/rfc/rfc7230#section-3.3.3
  // check if content-length and transfer-encoding are both present
  // todo(thara): refactor hasField
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

    // todo(thara): handle overflow
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

  // todo(thara): handle overflow
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
    DEBUG_PUTS("ss.fail()");
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

void validateCookieName(const std::string &name) {
  if (isToken(name)) return;
  throw BadRequestException("Http Request: invalid cookie name");
}

void validateCookieValue(const std::string &value) {
  for (std::string::const_iterator itr = value.begin(); itr != value.end(); itr++) {
    if (*itr == 0x21 || (0x23 <= *itr && *itr <= 0x2B) || (0x2D <= *itr && *itr <= 0x3A) ||
        (0x3C <= *itr && *itr <= 0x5B) || (0x5D <= *itr && *itr <= 0x7E)) {
      continue;
    }
    throw BadRequestException("Http Request: invalid cookie value");
  }
}

// this will not take obs-fold in OWS
void HttpRequestReader::analyzeCookie(const std::string &value) {
  insertIfNotDuplicate(CookieField, "Http Request: duplicated cookie");

  std::string::size_type start;
  // in order to have start 0 at the beginning
  std::string::size_type end = -2;
  while (end != std::string::npos) {
    start = end + 2;
    end = value.find("; ", start);
    std::string cookie = value.substr(start, end - start);
    std::string::size_type equalPos = cookie.find('=');
    if (equalPos == std::string::npos) {
      throw BadRequestException("Http Request: invalid cookie");
    }
    std::string cookieName = cookie.substr(0, equalPos);
    std::string cookieValue = cookie.substr(equalPos + 1);
    validateCookieName(cookieName);
    validateCookieValue(cookieValue);
  }
  request_.headers_.cookie_ = value;
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
      std::vector<char>::iterator end =
          std::search(raw_data_.begin(), raw_data_.end(), CRLFVec().begin(), CRLFVec().end());
      if (end == raw_data_.end()) {
        return;
      }

      std::string hex(raw_data_.begin(), end);
      if (!ishex(hex)) {
        throw BadRequestException("Http Request: invalid chunked body");
      }
      std::stringstream ss(hex);
      ss >> std::hex >> chunked_size_;

      if (chunked_size_ == 0) {
        rest_ = std::vector<char>(end + 2, raw_data_.end());
        moveToNextState();
        return;
      }

      raw_data_ = std::vector<char>(end + 2, raw_data_.end());
      chunked_reading_state_ = ReadingChunkedData;
    }

    if (chunked_reading_state_ == ReadingChunkedData) {
      if (raw_data_.size() < (chunked_size_ + 2)) {
        return;
      }
      if (raw_data_[chunked_size_] != '\r' || raw_data_[chunked_size_ + 1] != '\n') {
        throw BadRequestException("Http Request: invalid chunked body");
      }

      request_.body_.insert(request_.body_.end(), raw_data_.begin(), raw_data_.begin() + chunked_size_);
      raw_data_ = std::vector<char>(raw_data_.begin() + chunked_size_ + 2, raw_data_.end());

      chunked_reading_state_ = ReadingChunkedSize;
    }
  }
  return;
}

void HttpRequestReader::readBody() {
  request_.body_ = std::vector<char>(raw_data_.begin(), raw_data_.begin() + request_.headers_.content_length);
  moveToNextState();
}

bool HttpRequestReader::isActionable() {
  if (state_ == ReadingStartLine || state_ == ReadingHeaders) {
    return std::search(raw_data_.begin(), raw_data_.end(), CRLFVec().begin(), CRLFVec().end()) != raw_data_.end();
  } else if (state_ == ReadingBody) {
    // this accepts body that is larger than content-length
    return raw_data_.size() >= request_.headers_.content_length;
  } else {
    throw std::runtime_error("invalid HttpRequestReader state");
  }
}

// isActionable must be called before calling this function
void HttpRequestReader::trimToEndingChars() {
  std::vector<char>::iterator end_pos;
  if (state_ == ReadingStartLine) {
    end_pos = std::search(raw_data_.begin(), raw_data_.end(), CRLFVec().begin(), CRLFVec().end()) + CRLFVec().size();
  } else if (state_ == ReadingHeaders) {
    end_pos = std::search(raw_data_.begin(), raw_data_.end(), CRLFCRLFVec().begin(), CRLFCRLFVec().end());

    if (end_pos == raw_data_.end()) {
      std::vector<char>::reverse_iterator r_end_pos =
          std::search(raw_data_.rbegin(), raw_data_.rend(), CRLFVec().rbegin(), CRLFVec().rend());
      end_pos = r_end_pos.base();
    } else {
      end_pos += CRLFCRLFVec().size();
    }
  } else {
    throw std::runtime_error("invalid HttpRequestReader state");
  }

  rest_ = std::vector<char>(end_pos, raw_data_.end());
  raw_data_ = std::vector<char>(raw_data_.begin(), end_pos);
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
