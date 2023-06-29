#ifndef CGI_HPP_
#define CGI_HPP_

#include "../../EventManager.hpp"
#include "../Observee.hpp"

class CGI : public Observee {
 public:
  typedef std::pair<std::string, std::string> t_field;

  enum Type { Error, Doc, LocalRedir, ClientRedir, ClientRedirWithDoc };
  CGI(int id, int pid, EventManager *em, Observee *parent, HttpRequest *request, HttpResponse *response)
      : Observee(id, "cgi", em, parent),
        pid_(pid),
        request_(request),
        response_(response),
        sending_size_(0),
        recieved_size_(0) {}
  ~CGI() {}
  void notify(struct kevent ev);
  void shutdown();
  void timeout();
  void terminate();
  void handleCGIResponse();
  void parseHeaders(std::string &headers);
  Type getResponseType();
  bool isDocRes();
  bool isLocalRedirectRes();
  bool isClientRedirectRes();
  bool isClientRedirectWithDocRes();
  void processDocRes(std::string &body);
  void processClientRedirect();
  void processLocalRedirect();
  void processClientRedirectWithDoc(std::string &body);

 private:
  pid_t pid_;
  HttpRequest *request_;
  HttpResponse *response_;
  HttpResponse::t_headers headers_;
  std::string recieved_data_;
  std::size_t sending_size_;
  std::size_t recieved_size_;
};

#endif