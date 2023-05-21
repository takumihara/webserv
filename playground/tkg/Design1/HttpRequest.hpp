#ifndef HTTP_REQUEST_HPP_
#define HTTP_REQUEST_HPP_
#include <sys/event.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>

class EventManager;

class HttpRequest {
 public:
  enum State { Free, ReadStartLine, ReadHeaders, ReadBody, Processing };
  struct RequestLine {
    std::string method;
    std::string requestTarget;
    std::string version;
  };

  HttpRequest(int fd) : fd_(fd), state_(Free) { headers_["host"] = "localhost"; }
  ~HttpRequest(){};
  void readRequest(EventManager &em);
  void refresh();
  const std::string &getBody() const;

  // private:
  int fd_;
  std::string raw_data_;
  std::string rest_;
  State state_;

  RequestLine request_line_;
  // todo: lowercase key
  std::map<std::string, std::string> headers_;
  std::string body_;

  static const int kReadSize = 3;

  std::string getEndingChars() const;
  bool trimToEndingChars();
  void moveToNextState();
};

#endif
