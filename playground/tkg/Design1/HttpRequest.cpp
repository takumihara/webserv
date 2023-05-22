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
              << ":'" << raw_data_ << "'" << std::endl;

    while (trimToEndingChars()) {
      moveToNextState();

      std::cout << "request received"
                << "(fd:" << sock_fd_ << "): '" << raw_data_ << "'" << std::endl;

      if (state_ == ReadStartLine) {
        parseStartline();
        refresh();
      } else if (state_ == ReadHeaders) {
        parseHeaders();
        // todo: handle when body doesn't exist
        refresh();
        if (!isReceivingBody()) {
          finishedReading = true;
          break;
        }
      } else if (state_ == ReadBody) {
        body_ = raw_data_;
        finishedReading = true;
        break;
      }
    }
  }

  if (finishedReading) {
    state_ = Free;
    event_manager.addChangedEvents((struct kevent){sock_fd_, EVFILT_READ, EV_DISABLE, 0, 0, 0});
    event_manager.addChangedEvents((struct kevent){sock_fd_, EVFILT_TIMER, EV_DISABLE, 0, 0, NULL});
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

bool HttpRequest::trimToEndingChars() {
  size_t end_pos;
  const std::string ending_chars = getEndingChars();

  if ((end_pos = raw_data_.find(ending_chars)) != std::string::npos) {
    rest_ = raw_data_.substr(end_pos + ending_chars.size());
    raw_data_ = raw_data_.substr(0, end_pos);
  }
  return end_pos != std::string::npos;
}

std::string HttpRequest::getEndingChars() const {
  switch (state_) {
    case Free:
      return CRLF;
    case ReadStartLine:
      return std::string(CRLF) + CRLF;
    case ReadHeaders:
      return CRLF;
    default:
      throw std::runtime_error("invalid HttpRequest state");
  }
}

void HttpRequest::moveToNextState() {
  switch (state_) {
    case Free:
      state_ = ReadStartLine;
      break;
    case ReadStartLine:
      state_ = ReadHeaders;
      break;
    case ReadHeaders:
      state_ = ReadBody;
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
