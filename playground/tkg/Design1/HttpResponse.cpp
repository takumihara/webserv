#include "HttpResponse.hpp"

#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "EventManager.hpp"

void HttpResponse::creatingResponse() { response_ = raw_data_; }

void HttpResponse::sendResponse(EventManager &event_manager) {
  creatingResponse();
  const char *response = response_.c_str();
  std::cout << "sending response \n";
  int size = response_size_ - sending_response_size_;
  if (size > kWriteSize) {
    size = kWriteSize;
  }
  int res = sendto(fd_, &response[sending_response_size_], size, 0, NULL, 0);
  if (res == -1) {
    perror("sendto");
    throw std::runtime_error("send error");
  }
  std::string res_str = std::string(response);
  std::cout << "response sent: "
            << "'" << res_str.substr(sending_response_size_, size) << "'"
            << " (size:" << res << ")" << std::endl;
  sending_response_size_ += size;
  if (sending_response_size_ == response_size_) {
    refresh(event_manager);
  }
}

void HttpResponse::refresh(EventManager &em) {
  sending_response_size_ = 0;
  response_size_ = 0;
  em.addChangedEvents((struct kevent){fd_, EVFILT_WRITE, EV_DISABLE, 0, 0, 0});
  em.addChangedEvents((struct kevent){fd_, EVFILT_READ, EV_ENABLE, 0, 0, 0});
  response_ = "";
  raw_data_ = "";
}
