#ifndef HTTP_SERVER_HPP_
#define HTTP_SERVER_HPP_
#include <set>

#include "./Config/Config.hpp"
#include "EventManager.hpp"

class HttpServer {
 public:
  HttpServer(){};
  ~HttpServer(){};
  int openPort();
  void setup();
  void start();

 private:
  EventManager em_;
  Config conf_;
};

#endif