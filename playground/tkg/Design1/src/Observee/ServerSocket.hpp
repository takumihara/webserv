#ifndef SERVER_SOCKET_HPP_
#define SERVER_SOCKET_HPP_

#include "../EventManager.hpp"
#include "Observee.hpp"

class ServerSocket : public Observee {
 public:
  ServerSocket(int id, int port, Config &conf, EventManager *em)
      : Observee(id, "server", em, NULL), port_(port), conf_(conf) {}
  ~ServerSocket() {}
  void notify(struct kevent ev);
  void shutdown();
  void terminate();

 private:
  int port_;
  Config &conf_;
};

#endif