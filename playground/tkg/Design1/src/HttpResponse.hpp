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
      : sock_fd_(fd), port_(port), status_(0), body_(""), response_(""), response_size_(0), sending_response_size_(0) {}
  ~HttpResponse(){};
  void createResponse();
  void sendResponse(EventManager &em);
  void refresh(EventManager &em);
  void setStatus(const int status);
  void appendBody(const std::string &str);
  const std::string &getBody() const;

 private:
  int sock_fd_;
  int port_;
  int status_;
  // Config &conf_;
  std::string body_;
  std::string response_;
  int response_size_;
  int sending_response_size_;
};

#endif