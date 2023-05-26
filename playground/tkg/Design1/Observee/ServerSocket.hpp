#ifndef SERVER_SOCKET_HPP_
#define SERVER_SOCKET_HPP_

#include "../EventManager.hpp"
#include "Observee.hpp"

class ServerSocket : public Observee {
 public:
  ServerSocket(int id, int port, Config &conf) : Observee(id, "server", NULL), port_(port), conf_(conf) {}
  ~ServerSocket() {}
  void notify(EventManager &event_manager, struct kevent ev);
  void shutdown(EventManager &em);

 private:
  int port_;
  Config &conf_;
};

#endif