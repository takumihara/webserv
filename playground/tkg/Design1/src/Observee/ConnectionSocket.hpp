#ifndef CONNECTION_SOCKET_HPP_
#define CONNECTION_SOCKET_HPP_

#include <deque>

#include "../EventManager.hpp"
#include "../IO/IReadCloser.hpp"
#include "Observee.hpp"

class CGI;
class GET;

class ConnectionSocket : public Observee {
 public:
  ConnectionSocket(int id, int port, Config &conf, EventManager *em, Observee *parent, IReadCloser *rc)
      : Observee(id, "connection", em, parent),
        port_(port),
        conf_(conf),
        loc_conf_(NULL),
        request_(HttpRequest(id, &conf)),
        response_(HttpResponse(id, port, &conf)),
        rc_(rc) {}
  ~ConnectionSocket() { delete rc_; }
  void notify(struct kevent ev);
  void shutdown();
  void process();
  void processGET();
  void processErrorPage(const LocationConf *conf);
  void execCGI(const std::string &path);
  CGI *makeCGI(int id, int pid);
  GET *makeGET(int id);
  void addChild(Observee *obs);
  void removeChild(Observee *obs);

 private:
  int port_;
  Config &conf_;
  LocationConf *loc_conf_;
  HttpRequest request_;
  HttpResponse response_;
  IReadCloser *rc_;
  Cache cache_;
  std::string extension_;

  //   std::deque<HttpRequest> request_;
  //   std::deque<HttpResponse> response_;
};

#endif
