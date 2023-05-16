#ifndef CONNECTION_SOCKET_HPP_
#define CONNECTION_SOCKET_HPP_

#include "debug.hpp"

class EventManager;

class ConnectionSocket {
 public:
  enum SocketState {
    kSocFree,
    kSocReading,
    kSocWriting,
  };
  ConnectionSocket(int fd);
  ~ConnectionSocket() {}
  void handle_request(EventManager &event_manager);
  void notify(EventManager &event_manager);
  void send_response(int socket_fd, char *response);

 private:
  int fd_;
  SocketState state_;
};

#endif
