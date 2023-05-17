#ifndef HTTP_REQUEST_HPP_
#define HTTP_REQUEST_HPP_
#include <sys/event.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <string>

class EventManager;

class HttpRequest {
 public:
  HttpRequest(int fd) : fd_(fd) {}
  ~HttpRequest(){};
  void readRequest(EventManager &em);
  void refresh();

  // private:
  int fd_;
  std::string request_;
  std::string rest;
  static const int kReadSize = 3;
};

#endif
