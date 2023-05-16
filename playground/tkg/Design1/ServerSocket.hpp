#ifndef SERVER_SOCKET_HPP_
#define SERVER_SOCKET_HPP_

#include <map>

class EventManager;

class ServerSocket {
 public:
  ServerSocket(int fd) : fd_(fd) {}
  ~ServerSocket() {}
  void notify(EventManager &event_manager);
  void make_client_connection(EventManager &event_manager);

 private:
  int fd_;
};

#endif
