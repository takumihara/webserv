#ifndef CONNECTION_SOCKET_HPP_
#define CONNECTION_SOCKET_HPP_

#include "../EventManager.hpp"
#include "../IO/IReadCloser.hpp"
// #include "CGI.hpp"
#include <deque>

#include "Observee.hpp"
class CGI;

class ConnectionSocket : public Observee {
 public:
  ConnectionSocket(int id, int port, Config &conf, Observee *parent, IReadCloser *rc)
      : Observee(id, "connection", parent),
        port_(port),
        conf_(conf),
        request_(HttpRequest(id, port, conf)),
        response_(HttpResponse(id, port)),
        rc_(rc) {}
  ~ConnectionSocket() { delete rc_; }
  void notify(EventManager &event_manager, struct kevent ev);
  void shutdown(EventManager &em);
  void send_response(EventManager &event_manager);
  void process(EventManager &em);
  void processGET(const LocationConf &loc_conf, std::string path);
  void execCGI(const std::string &path, EventManager &event_manager);
  CGI *makeCGI(int id, int pid);
  std::string getTargetPath(const LocationConf &loc);
  std::string listFilesAndDirectories(const std::string &directoryPath);
  std::string getIndexFile(const LocationConf &conf, std::string path);
  void addChild(Observee *obs);
  void removeChild(Observee *obs);

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
