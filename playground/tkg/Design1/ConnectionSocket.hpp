#ifndef CONNECTION_SOCKET_HPP_
#define CONNECTION_SOCKET_HPP_

#include <string>

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
  void handle_response(EventManager &event_manager);
  void notify(EventManager &event_manager);
  void send_response(EventManager &event_manager, int socket_fd, const char *response);

 private:
  int fd_;
  SocketState state_;
  int response_size_;
  int sending_response_size_;
  std::string response_;
  static const int kWriteSize = 3;
};

#endif
