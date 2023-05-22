#include "HttpRequest.hpp"

#include <algorithm>
#include <sstream>

#include "EventManager.hpp"
#include "const.hpp"
#include "helper.hpp"

const std::string HttpRequest::kSupportedMethods[] = {"GET", "POST"};
const std::string HttpRequest::kSupportedVersions[] = {"HTTP/1.1"};

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
  return request_line_.method != "GET";
}

void HttpRequest::parseStartline() {
  std::stringstream ss = std::stringstream(raw_data_);

  std::getline(ss, request_line_.method, SP);
  std::getline(ss, request_line_.requestTarget, SP);
  ss >> request_line_.version;

  validateStartLine();

  DEBUG_PUTS("REQUEST LINE PARSED");
  DEBUG_PRINTF("method: '%s' request target: '%s' version: '%s'\n", request_line_.method.c_str(),
               request_line_.requestTarget.c_str(), request_line_.version.c_str());
}

void HttpRequest::validateStartLine() {
  if (!isValidMethod()) {
    throw std::runtime_error("Http Request: invalid method");
  }
  if (!isValidRequestTarget()) {
    throw std::runtime_error("Http Request: invalid request target");
  }
  if (!isValidVersion()) {
    throw std::runtime_error("Http Request: invalid version");
  }
}

bool HttpRequest::isValidMethod() {
  return in(request_line_.method, kSupportedMethods, sizeof(kSupportedMethods) / sizeof(std::string));
}

bool HttpRequest::isValidRequestTarget() {
  // todo: add validation
  return request_line_.requestTarget[0] == '/';
}

bool HttpRequest::isValidVersion() {
  return in(request_line_.version, kSupportedVersions, sizeof(kSupportedVersions) / sizeof(std::string));
}

void HttpRequest::parseHeaders() {
  std::stringstream ss(raw_data_);

  std::string::size_type start = 0;
  std::string::size_type end = raw_data_.find("\r\n", start);

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
        validateHeaderValue(value);

        if (headers_.find(name) == headers_.end()) {
          headers_[name] = value;
        } else {
          headers_[name] += "," + value;
        }
      }
    }

    start = end + 2;  // Skip the \r\n
    end = raw_data_.find("\r\n", start);
  }

  DEBUG_PUTS("HEADER PARSED");
  for (std::map<std::string, std::string>::iterator itr = headers_.begin(); itr != headers_.end(); itr++) {
    DEBUG_PRINTF("'%s' : '%s'\n", itr->first.c_str(), itr->second.c_str());
  }
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
  raw_data_ = raw_data_.substr(0, end_pos);
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
      if (getHeaderValue("transfer-encoding") == "chunked") {
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

const std::string &HttpRequest::getRequestTarget() const { return request_line_.requestTarget; }
const std::string &HttpRequest::getHeaderValue(const std::string &name) const { return headers_.find(name)->second; }
