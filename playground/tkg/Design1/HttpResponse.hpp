#ifndef HTTP_RESPONSE_HPP_
#define HTTP_RESPONSE_HPP_
#include <sys/event.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <string>

class EventManager;

class HttpResponse {
 public:
  HttpResponse(int fd) : fd_(fd), raw_data_(""), response_(""), response_size_(0), sending_response_size_(0) {}
  ~HttpResponse(){};
  void creatingResponse();
  void sendResponse(EventManager &em);
  void refresh(EventManager &em);

  // private:
  int fd_;
  std::string raw_data_;
  std::string response_;
  int response_size_;
  int sending_response_size_;
  static const int kWriteSize = 100;
};

#endif
