#ifndef HTTP_SERVER_HPP_
#define HTTP_SERVER_HPP_
#include <set>

class HttpServer {
 public:
  static HttpServer &getInstance();

 private:
  HttpServer();
  HttpServer(const HttpServer &server);
  void operator=(const HttpServer &from);

 private:
};

#endif