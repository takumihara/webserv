#ifndef GET_HPP_
#define GET_HPP_

#include "../EventManager.hpp"
#include "Observee.hpp"

class GET : public Observee {
 public:
  GET(int id, EventManager *em, Observee *parent, HttpResponse *response)
      : Observee(id, "GET", em, parent), response_(response) {}
  ~GET() {}
  void notify(struct kevent ev);
  void shutdown();
  void terminate();
  static std::string listFilesAndDirectories(const std::string &directory_path);

 private:
  HttpResponse *response_;
};

#endif