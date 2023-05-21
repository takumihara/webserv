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
  ConnectionSocket(int fd, Config &conf);
  ~ConnectionSocket() {}
  void handle_request(EventManager &event_manager);
  void handle_response(EventManager &event_manager, Config &conf);
  void notify(EventManager &event_manager);
  void send_response(EventManager &event_manager);
  void setToReadingState(EventManager &em);
  std::string process(Config &conf);

 private:
  int fd_;
  Config conf_;
  // SocketState state_;
  HttpRequest request_;
  HttpResponse response_;
};

#endif
