#ifndef HTTP_SERVER_HPP_
#define HTTP_SERVER_HPP_
#include <set>

#include "EventManager.hpp"

class HttpServer {
 public:
  HttpServer(){};
  ~HttpServer(){};
  int openPort();
  void start();

 private:
  EventManager em_;
};

#endif