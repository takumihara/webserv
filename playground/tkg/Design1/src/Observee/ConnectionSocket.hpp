#ifndef CONNECTION_SOCKET_HPP_
#define CONNECTION_SOCKET_HPP_

#include "../EventManager.hpp"
#include "../IO/IReadCloser.hpp"
// #include "CGI.hpp"
#include <deque>

#include "Observee.hpp"
class CGI;
class GET;

class ConnectionSocket : public Observee {
 public:
  ConnectionSocket(int id, int port, Config &conf, EventManager *em, Observee *parent, IReadCloser *rc)
      : Observee(id, "connection", em, parent),
        port_(port),
        conf_(conf),
        request_(HttpRequest(id, port, conf)),
        response_(HttpResponse(id, port)),
        rc_(rc) {}
  ~ConnectionSocket() { delete rc_; }
  void notify(struct kevent ev);
  void shutdown();
  void process();
  void processGET(const ServerConf *serv_conf);
  void execCGI(const ServerConf *serv_conf);
  CGI *makeCGI(int id, int pid);
  GET *makeGET(int id);
  std::string getTargetPath(const LocationConf &loc);
  std::string getIndexFile(const LocationConf &conf, std::string path);
  void addChild(Observee *obs);
  void removeChild(Observee *obs);
  void disableReadAndAddWriteEvent(uintptr_t read, uintptr_t write);
  void disableReadAndAddReadEvent(uintptr_t parent, uintptr_t child);

 private:
  int port_;
  Config &conf_;
  // std::string result_;
  HttpRequest request_;
  HttpResponse response_;
  IReadCloser *rc_;
  //   std::deque<HttpRequest> request_;
  //   std::deque<HttpResponse> response_;
};

#endif
