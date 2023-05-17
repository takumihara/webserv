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
  enum State { Free, ReadStartLine, ReadHeaders, ReadBody, Processing };

  HttpRequest(int fd) : fd_(fd), state_(Free) {}
  ~HttpRequest(){};
  void readRequest(EventManager &em);
  void refresh();
  std::string getEndingChars() const;
  void moveToNextState();
  bool trimToEndingChars();

  // private:
  int fd_;
  std::string raw_data_;
  std::string rest_;
  State state_;

  static const int kReadSize = 3;
};

#endif
