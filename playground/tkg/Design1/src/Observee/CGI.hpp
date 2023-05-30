#ifndef CGI_HPP_
#define CGI_HPP_

#include "../EventManager.hpp"
#include "Observee.hpp"

class CGI : public Observee {
 public:
  CGI(int id, int pid, Observee *parent, std::string *result)
      : Observee(id, "cgi", parent), pid_(pid), result_(result) {}
  ~CGI() {}
  void notify(EventManager &event_manager, struct kevent ev);
  void shutdown(EventManager &em);

 private:
  int pid_;
  std::string *result_;
};

#endif