#ifndef POST_HPP_
#define POST_HPP_

#include "../EventManager.hpp"
#include "Observee.hpp"

class POST : public Observee {
 public:
  POST(int id, EventManager *em, Observee *parent, HttpRequest *request)
      : Observee(id, "POST", em, parent), request_(request), body_size_(request->body_.size()), write_size_(0) {}
  ~POST() {}
  void notify(struct kevent ev);
  void shutdown();
  void timeout();
  void terminate();

 private:
  HttpRequest *request_;
  std::size_t body_size_;
  std::size_t write_size_;
};

#endif
