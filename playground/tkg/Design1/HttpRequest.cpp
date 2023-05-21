#include "HttpRequest.hpp"

#include "EventManager.hpp"

void HttpRequest::readRequest(EventManager &event_manager) {
  char request[kReadSize + 1];
  bzero(request, kReadSize + 1);
  std::string req_str;
  int size = read(fd_, request, kReadSize);
  if (size == -1) {
    // todo: no exception
    throw std::runtime_error("read error");
  } else if (size == 0) {
    printf("closed fd = %d\n", fd_);
    close(fd_);
    event_manager.removeConnectionSocket(fd_);
  } else {
    req_str = std::string(request);
    raw_data_ += req_str;

    std::cout << "read from socket(fd:" << fd_ << ")"
              << ":" << raw_data_ << std::endl;

    while (trimToEndingChars()) {
      moveToNextState();

      std::cout << "request received"
                << "(fd:" << fd_ << "): '" << raw_data_ << "'" << std::endl;

      if (state_ == ReadStartLine) {
        // parseStartline(raw_data_);
        refresh();
        DEBUG_PUTS("parsing startline done");
      } else if (state_ == ReadHeaders) {
        // parseHeaders()
        // todo: handle when body doesn't exist
        refresh();
        DEBUG_PUTS("parsing headers done");
      } else if (state_ == ReadBody) {
        body_ = raw_data_;
        state_ = Free;
        event_manager.addChangedEvents((struct kevent){fd_, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0});
        event_manager.addChangedEvents((struct kevent){fd_, EVFILT_READ, EV_DISABLE, 0, 0, 0});
        DEBUG_PUTS("processing done");
        break;
      }
    }
  }
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
      return "\r\n";
    case ReadStartLine:
      return "\r\n\r\n";
    case ReadHeaders:
      return "\r\n";
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
