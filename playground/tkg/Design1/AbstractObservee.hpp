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

#include "./Config/Config.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "const.hpp"
#include "debug.hpp"

typedef uintptr_t t_id;
typedef short t_type;

class EventManager;
#define FD 1
#define PID 2

class AbstractObservee {
 public:
  AbstractObservee(int id, const std::string &type, AbstractObservee *parent) : id_(id), type_(type), parent_(parent) {}
  virtual ~AbstractObservee() {}
  virtual void notify(EventManager &event_manager, struct kevent ev) = 0;
  virtual void shutdown() = 0;
  virtual void obliviateChild(AbstractObservee *child) {
    if (parent_) parent_->children_.erase(child);
  };
  virtual void monitorChild(AbstractObservee *child) { children_.insert(child); };

 public:
  int id_;
  std::string type_;
  AbstractObservee *parent_;
  std::set<AbstractObservee *> children_;
};

class CGI;

class ConnectionSocket : public AbstractObservee {
 public:
  ConnectionSocket(int id, int port, Config &conf, AbstractObservee *parent)
      : AbstractObservee(id, "connection", parent),
        port_(port),
        conf_(conf),
        result_(""),
        request_(HttpRequest(id, port)),
        response_(HttpResponse(id, port)) {}
  ~ConnectionSocket() {}
  void notify(EventManager &event_manager, struct kevent ev);
  void shutdown();
  void send_response(EventManager &event_manager);
  void process(EventManager &em);
  void execCGI(const std::string &path, EventManager &event_manager);
  CGI *makeCGI(int id, int pid);
  void addChild(AbstractObservee *obs);
  void removeChild(AbstractObservee *obs);

 private:
  int port_;
  Config &conf_;
  std::string result_;
  HttpRequest request_;
  HttpResponse response_;
};

class ServerSocket : public AbstractObservee {
 public:
  ServerSocket(int id, int port, Config &conf) : AbstractObservee(id, "server", NULL), port_(port), conf_(conf) {}
  ~ServerSocket() {}
  void notify(EventManager &event_manager, struct kevent ev);
  void shutdown();

 private:
  int port_;
  Config &conf_;
};

class CGI : public AbstractObservee {
 public:
  CGI(int id, int pid, AbstractObservee *parent, std::string *result)
      : AbstractObservee(id, "cgi", parent), pid_(pid), result_(result) {}
  ~CGI() {}
  void notify(EventManager &event_manager, struct kevent ev);
  void shutdown();

 private:
  int pid_;
  std::string *result_;
};

#endif
