#ifndef HTTP_RESPONSE_HPP_
#define HTTP_RESPONSE_HPP_
#include <sys/event.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "./Config/Config.hpp"

class EventManager;

class HttpResponse {
 public:
  HttpResponse(int fd, int port)
      : sock_fd_(fd),
        port_(port),
        raw_data_(""),
        response_(""),
        response_size_(0),
        sending_response_size_(0) {}
  ~HttpResponse(){};
  void createResponse(const std::string &result);
  void sendResponse(EventManager &em);
  void refresh(EventManager &em);

 private:
  int sock_fd_;
  int port_;
  // Config &conf_;
  std::string raw_data_;
  std::string response_;
  int response_size_;
  int sending_response_size_;
};

#endif
