#ifndef CONNECTION_SOCKET_HPP_
#define CONNECTION_SOCKET_HPP_

#include "AbstractSocket.hpp"
#include "EventManager.hpp"

class ConnectionSocket : public AbstractSocket {
 public:
  enum SocketState {
    kSocFree,
    kSocReading,
    kSocWriting,
  };
  ConnectionSocket(int fd, SocketState state);
  ~ConnectionSocket() {}
  void handle_request(EventManager &event_manager);
  void notify(EventManager &event_manager);
  void send_response(int socket_fd, char *response);

 private:
  SocketState state_;
};

#endif
