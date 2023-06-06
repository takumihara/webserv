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
  void processGET(const LocationConf &loc_conf);
  void execCGI(const std::string &path);
  CGI *makeCGI(int id, int pid);
  GET *makeGET(int id);
  std::string getTargetPath(const LocationConf &loc);
  std::string getIndexFile(const LocationConf &conf, std::string path);
  void addChild(Observee *obs);
  void removeChild(Observee *obs);
  void disableReadAndAddWriteEvent(uintptr_t read, uintptr_t write);
  void disableReadAndAddReadEvent(uintptr_t parent, uintptr_t child);

  class HttpException : public std::runtime_error {
   public:
    HttpException(int statusCode, const std::string &statusMessage)
        : std::runtime_error(statusMessage), statusCode_(statusCode) {}
    int statusCode() const { return statusCode_; }

   private:
    int statusCode_;
  };

  class BadRequestException : public HttpException {
   public:
    BadRequestException(const std::string &message) : HttpException(400, message) {}
  };

  class ResourceNotFoundException : public HttpException {
   public:
    ResourceNotFoundException(const std::string &message) : HttpException(404, message) {}
  };
  class ResourceForbidenException : public HttpException {
   public:
    ResourceForbidenException(const std::string &message) : HttpException(403, message) {}
  };
  class InternalServerErrorException : public HttpException {
   public:
    InternalServerErrorException(const std::string &message) : HttpException(500, message) {}
  };

 private:
  int port_;
  Config &conf_;
  HttpRequest request_;
  HttpResponse response_;
  IReadCloser *rc_;
  std::string extension_;

  //   std::deque<HttpRequest> request_;
  //   std::deque<HttpResponse> response_;
};

#endif
