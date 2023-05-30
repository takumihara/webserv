#ifndef HTTP_SERVER_HPP_
#define HTTP_SERVER_HPP_
#include <set>

#include "./Config/Config.hpp"
#include "EventManager.hpp"

class HttpServer {
 public:
  HttpServer() : filename_("Config/con.conf"){};
  HttpServer(char *filename) : filename_(filename){};
  ~HttpServer(){};
  int openPort();
  void setup();
  void start();

 private:
  EventManager em_;
  const char *filename_;
  Config conf_;
};

#endif