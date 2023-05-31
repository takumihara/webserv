#ifndef CGI_HPP_
#define CGI_HPP_

#include "../EventManager.hpp"
#include "Observee.hpp"

class CGI : public Observee {
 public:
  CGI(int id, int pid, EventManager *em, Observee *parent, HttpResponse *response)
      : Observee(id, "cgi", em, parent), pid_(pid), response_(response) {}
  ~CGI() {}
  void notify(struct kevent ev);
  void shutdown();

 private:
  int pid_;
  HttpResponse *response_;
};

#endif