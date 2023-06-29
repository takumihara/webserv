#ifndef ABSTRACT_OBSERVEE_HPP_
#define ABSTRACT_OBSERVEE_HPP_

#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <set>

#include "../Config/Config.hpp"
#include "../HttpResponse.hpp"
#include "../const.hpp"
#include "../debug.hpp"
#include "HttpRequest.hpp"

typedef uintptr_t t_id;
typedef short t_type;

class EventManager;
#define FD 1
#define PID 2

class Observee {
 public:
  Observee(int id, const std::string &type, EventManager *em, Observee *parent)
      : id_(id), type_(type), em_(em), parent_(parent) {}
  virtual ~Observee() {}
  virtual void notify(struct kevent ev) = 0;
  virtual void shutdown() = 0;
  virtual void timeout() = 0;
  virtual void terminate() = 0;
  void stopMonitorChild(Observee *child) { children_.erase(std::find(children_.begin(), children_.end(), child)); };
  void monitorChild(Observee *child) { children_.push_back(child); };

 public:
  uintptr_t id_;
  std::string type_;
  EventManager *em_;
  Observee *parent_;
  std::vector<Observee *> children_;
};

#endif
