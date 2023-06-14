#ifndef CGI_HPP_
#define CGI_HPP_

#include "../EventManager.hpp"
#include "Observee.hpp"

class CGI : public Observee {
 public:
  enum Type { Doc, LocalRedir, ClientRedir, ClientRedirWithDoc };
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
  int parseCGIResponse();
  Type getResponseType(std::vector<std::string> &lines);
  bool isDocRes(std::vector<std::string> &line);
  bool isLocalRedirectRes(std::vector<std::string> &line);
  bool isClientRedirectRes(std::vector<std::string> &line);
  bool isClientRedirectWithDocRes(std::vector<std::string> &line);

 private:
  int pid_;
  HttpRequest *request_;
  HttpResponse *response_;
  std::string recieve_data_;
  std::size_t sending_size_;
  std::size_t recieved_size_;
};

#endif