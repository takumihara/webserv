#ifndef ABSTRACT_SOCKET_HPP_
#define ABSTRACT_SOCKET_HPP_

#include "debug.hpp"

class EventManager;

class AbstractSocket {
 public:
  AbstractSocket(int fd) : fd_(fd) {}
  virtual ~AbstractSocket() {}
  virtual void notify(EventManager &event_manager) = 0;

 protected:
 public:
  int fd_;
};

#endif
