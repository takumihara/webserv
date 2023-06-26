#ifndef HTTP_RESPONSE_HPP_
#define HTTP_RESPONSE_HPP_
#include <sys/event.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "./Config/Config.hpp"

class EventManager;

class HttpResponse {
 public:
  typedef std::pair<std::string, std::string> header;
  enum State { Free, Sending, End };

  HttpResponse(int fd, int port, Config *conf)
      : sock_fd_(fd),
        state_(Free),
        port_(port),
        status_(0),
        conf_(conf),
        response_size_(0),
        sending_response_size_(0) {}
  ~HttpResponse(){};
  void createResponse();
  void sendResponse();
  void refresh();
  int getStatus() const;
  void setStatus(const int status);
  void setStatusAndReason(const int status, const std::string &reason);
  void setContentType(const std::string &path);
  void appendHeader(const std::string &key, const std::string &value);
  void appendBody(const char *str, size_t size);
  void appendBody(const std::string &str);
  bool hasHeader(const std::string &name);
  const std::vector<char> &getBody() const;
  const State &getState() const { return state_; }

 private:
  int sock_fd_;
  State state_;
  int port_;
  int status_;
  std::string reason_phrase_;
  Config *conf_;
  std::vector<char> body_;
  std::vector<char> response_;
  std::vector<header> headers;
  int response_size_;
  int sending_response_size_;
};

#endif
