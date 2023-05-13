#ifndef SERVER_SOCKET_HPP_
#define SERVER_SOCKET_HPP_

#include <map>

#include "AbstractSocket.hpp"
#include "EventManager.hpp"

class ServerSocket : public AbstractSocket {
 public:
  ServerSocket(int fd) : AbstractSocket(fd) {}
  ~ServerSocket() {}
  void notify(EventManager &event_manager);
  void make_client_connection(EventManager &event_manager);
};

#endif
