#ifndef CONNECTION_SOCKET_HPP_
#define CONNECTION_SOCKET_HPP_

#include <deque>

// #include "../Config/Cache.hpp"
#include "../EventManager.hpp"
#include "../IO/IReadCloser.hpp"
#include "HttpRequestReader.hpp"
#include "Observee.hpp"

class CGI;
class GET;
class POST;

class ConnectionSocket : public Observee {
 public:
  ConnectionSocket(int id, int port, Config &conf, EventManager *em, Observee *parent, IReadCloser *rc)
      : Observee(id, "connection", em, parent),
        port_(port),
        conf_(conf),
        loc_conf_(NULL),
        rc_(rc),
        rreader_(id, &conf, request_, rc_),
        response_(HttpResponse(id, port, &conf)) {}
  ~ConnectionSocket() { delete rc_; }
  void notify(struct kevent ev);
  void shutdown();
  void timeout();
  void terminate();
  void process();
  void processGET();
  void processPOST();
  void processDELETE();
  void processErrorPage(const LocationConf *conf);
  void processRedirect();
  void execCGI(const std::string &path);
  CGI *makeCGI(int id, int pid);
  GET *makeGET(int id);
  void initExtension();
  HttpResponse *initResponse();
  // CGIInfo parseCGI(const std::string &path);

  void addChild(Observee *obs);
  void removeChild(Observee *obs);

 private:
  int port_;
  Config &conf_;
  LocationConf *loc_conf_;
  HttpRequest request_;
  IReadCloser *rc_;
  HttpRequestReader rreader_;
  HttpResponse response_;
  Cache cache_;
  std::string extension_;

  //   std::deque<HttpRequest> request_;
  //   std::deque<HttpResponse> response_;
};

#endif
