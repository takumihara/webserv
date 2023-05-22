#ifndef HTTP_REQUEST_HPP_
#define HTTP_REQUEST_HPP_
#include <sys/event.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>

#include "./Config/Config.hpp"

#define SP ' '
#define CRLF "\r\n"

class EventManager;

class HttpRequest {
 public:
  enum State { Free, ReadStartLine, ReadHeaders, ReadBody, Processing };
  struct RequestLine {
    std::string method;
    std::string requestTarget;
    std::string version;
  };

  HttpRequest(int fd, int port, Config &conf) : sock_fd_(fd), port_(port), conf_(conf), state_(Free) {}
  ~HttpRequest(){};
  void readRequest(EventManager &em);
  void refresh();
  const std::string &getBody() const;
  const std::string &getHeaderValue(const std::string &name) const;
  const std::string &getRequestTarget() const;

  // private:
  int sock_fd_;
  int port_;
  Config &conf_;
  std::string raw_data_;
  std::string rest_;
  State state_;

  RequestLine request_line_;
  // todo: lowercase key
  std::map<std::string, std::string> headers_;
  std::string body_;

  static const int kReadSize = 3;
  static const std::string kSupportedMethods[];
  static const std::string kSupportedVersions[];

  std::string getEndingChars() const;
  bool trimToEndingChars();
  void moveToNextState();

  void parseStartline();
  void validateStartLine();
  bool isValidMethod();
  bool isValidVersion();
  bool isValidRequestTarget();

  void parseHeaders();
  void validateHeaderName(const std::string &name);
  void validateHeaderValue(const std::string &value);

  bool isReceivingBody();
};

#endif
