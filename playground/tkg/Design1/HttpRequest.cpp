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
    size_t end_pos;
    if ((end_pos = raw_data_.find("\r\n\r\n")) == std::string::npos) {
      std::cout << "request too long (fd:" << fd_ << ")"
                << ":" << raw_data_ << std::endl;
    } else {
      rest_ = raw_data_.substr(end_pos + 4);
      std::cout << "request received"
                << "(fd:" << fd_ << "): '" << raw_data_ << "'" << std::endl;
      event_manager.addChangedEvents((struct kevent){fd_, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0});
      event_manager.addChangedEvents((struct kevent){fd_, EVFILT_READ, EV_DISABLE, 0, 0, 0});
    }
  }
}

void HttpRequest::refresh() {
  raw_data_ = rest_;
  rest_ = "";
}
