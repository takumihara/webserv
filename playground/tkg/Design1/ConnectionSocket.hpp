#ifndef CONNECTION_SOCKET_HPP_
#define CONNECTION_SOCKET_HPP_

#include <string>

#include "./Config/Config.hpp"
#include "EventManager.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "debug.hpp"

class EventManager;
class ConnectionSocket {
 public:
  // enum SocketState {
  //   kSocFree,
  //   kSocReading,
  //   kSocWriting,
  // };
  ConnectionSocket(int fd, int port, Config &conf);
  ~ConnectionSocket() {}
  bool handle_request(EventManager &event_manager);
  void handle_response(EventManager &event_manager);
  void handleCGI(EventManager &event_manager, int cgi_fd);
  void notify(EventManager &event_manager);
  void send_response(EventManager &event_manager);
  void setToReadingState(EventManager &em);
  void process(EventManager &event_manager);
  void execCgi(const std::string &path, EventManager &event_manager);

 private:
  int sock_fd_;
  int port_;
  Config &conf_;
  // SocketState state_;
  std::string result_;
  HttpRequest request_;
  HttpResponse response_;
};

#endif
