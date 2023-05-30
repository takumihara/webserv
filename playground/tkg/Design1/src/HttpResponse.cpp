#include "HttpResponse.hpp"

#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "EventManager.hpp"
#include "const.hpp"

void HttpResponse::createResponse(const std::string &result) {
  raw_data_ = result;
  response_ = result;
  // response_ = "HTTP/1.1 200 Ok\r\nContent-Length: " + std::to_string(result.size()) + "\r\n\r\n" + result + "\r\n";
  response_size_ = response_.size();
}

void HttpResponse::sendResponse(EventManager &event_manager) {
  const char *response = response_.c_str();
  std::cout << "sending response \n";
  int size = response_size_ - sending_response_size_;
  if (size > SOCKET_WRITE_SIZE) {
    size = SOCKET_WRITE_SIZE;
  }
  int res = sendto(sock_fd_, &response[sending_response_size_], size, 0, NULL, 0);
  if (res == -1) {
    perror("sendto");
    throw std::runtime_error("send error");
  }
  std::string res_str = std::string(response);
  std::cout << "response sent: "
            << "'" << res_str.substr(sending_response_size_, size) << "'"
            << " (size:" << res << ")" << std::endl;
  sending_response_size_ += size;
  std::cout << "response size: " << response_size_ << "(" << sending_response_size_ << std::endl;
  if (sending_response_size_ == response_size_) {
    refresh(event_manager);
  }
}

void HttpResponse::refresh(EventManager &em) {
  sending_response_size_ = 0;
  response_size_ = 0;
  (void)port_;
  em.addChangedEvents((struct kevent){static_cast<uintptr_t>(sock_fd_), EVFILT_WRITE, EV_DISABLE, 0, 0, 0});
  em.addChangedEvents((struct kevent){static_cast<uintptr_t>(sock_fd_), EVFILT_READ, EV_ENABLE, 0, 0, 0});
  // em.addChangedEvents(
  //    (struct kevent){sock_fd_, EVFILT_TIMER, EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
  response_ = "";
  raw_data_ = "";
}
