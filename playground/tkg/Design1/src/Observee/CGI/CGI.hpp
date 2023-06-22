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
  void terminate();
  void parseCGIResponse();
  Type getResponseType(std::vector<std::string> &lines);
  bool isDocRes(std::vector<std::string> &line);
  bool isLocalRedirectRes(std::vector<std::string> &line);
  bool isClientRedirectRes(std::vector<std::string> &line);
  bool isClientRedirectWithDocRes(std::vector<std::string> &line);
  void parseDocRes(std::vector<std::string> &lines);
  void parseClientRedirect(std::vector<std::string> &lines);
  void parseLocalRedirect(std::vector<std::string> &lines);
  void parseClientRedirectWithDoc(std::vector<std::string> &lines);

 private:
  pid_t pid_;
  HttpRequest *request_;
  HttpResponse *response_;
  std::string recieved_data_;
  std::size_t sending_size_;
  std::size_t recieved_size_;
};

#endif