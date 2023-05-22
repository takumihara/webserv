#include "EventManager.hpp"

#include "./Config/Config.hpp"
#include "AbstractObservee.hpp"
#include "debug.hpp"

EventManager::EventManager() {
  kq_ = kqueue();
  if (kq_ == -1) {
    throw std::runtime_error("kqueue error");
  }
}

void EventManager::addChangedEvents(struct kevent kevent) { changed_events_.push_back(kevent); }

void EventManager::add(const std::pair<t_id, t_type> &key, AbstractObservee *obs) { observees_[key] = obs; }

void EventManager::remove(const std::pair<t_id, t_type> &key) { observees_[key]->shutdown(); }

void EventManager::registerServerEvent(int fd, int port, Config &conf) {
  struct kevent chlist;
  bzero(&chlist, sizeof(struct kevent));
  DEBUG_PRINTF("server fd: %d, ", fd);
  EV_SET(&chlist, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
  DEBUG_PUTS("");
  kevent(kq_, &chlist, 1, NULL, 0, NULL);
  AbstractObservee *obs = new ServerSocket(fd, port, conf);
  std::cout << "sock fd: " << fd << std::endl;
  add(std::pair<t_id, t_type>(fd, FD), obs);
  observees_[std::pair<t_id, t_type>(fd, FD)];
  std::cout << observees_[std::pair<t_id, t_type>(fd, FD)]->id_;
}

std::string getEventFilter(int flag) {
  switch (flag) {
    case EVFILT_READ:
      return "EVFILT_READ";
    case EVFILT_WRITE:
      return "EVFILT_WRITE";
    case EVFILT_TIMER:
      return "EVFILT_TIMER";
    default:
      return "UNKNOWN";
  }
}

std::string getEventFlags(int state) {
  std::string res;

  if (state & EV_ADD) res += "EV_ADD, ";
  if (state & EV_DELETE) res += "EV_DELETE, ";
  if (state & EV_ENABLE) res += "EV_ENABLE, ";
  if (state & EV_DISABLE) res += "EV_DISABLE, ";

  return res;
}

void EventManager::updateKqueue() {
  DEBUG_PUTS("UPDATE KQUEUE");
  int size = changed_events_.size();
  struct kevent chlist[size];
  bzero(chlist, sizeof(struct kevent) * size);
  int i = 0;
  for (changed_events_const_iterator itr = changed_events_.begin(); itr != changed_events_.end(); itr++, i++) {
    DEBUG_PRINTF("fd: %lu(%s:%s), ", (*itr).ident, getEventFilter((*itr).filter).c_str(),
                 getEventFlags((*itr).flags).c_str());
    EV_SET(&chlist[i], (*itr).ident, (*itr).filter, (*itr).flags, (*itr).fflags, (*itr).data, (*itr).udata);
  }
  DEBUG_PUTS("");
  kevent(kq_, chlist, size, NULL, 0, NULL);
  changed_events_.clear();
}

t_type getType(short filter) {
  (void)filter;
  if (filter == EVFILT_READ || filter == EVFILT_WRITE)
    return FD;
  else if (filter == EVFILT_PROC)
    return PID;
  return 0;
}

void EventManager::handleEvent(struct kevent ev) {
  DEBUG_PUTS("START HANDLE EVENT");
  if (ev.filter == EVFILT_TIMER) {
    DEBUG_PUTS("timeout");
    handleTimeout(ev);
  } else {
    std::cout << "koko \n";
    observees_[std::pair<t_id, t_type>(ev.ident, getType(ev.filter))]->notify(*this, ev);
  }
  DEBUG_PUTS("END HANDLE EVENT");
}

void EventManager::handleTimeout(struct kevent ev) {
  close(ev.ident);
  // removeConnectionSocket(ev.ident);
  remove(std::pair<t_id, t_type>(ev.ident, FD));
  addChangedEvents((struct kevent){ev.ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
}

void EventManager::clearEvlist(struct kevent *evlist) { bzero(evlist, sizeof(struct kevent) * kMaxEventSize); }

void EventManager::eventLoop() {
  struct kevent evlist[kMaxEventSize];
  DEBUG_PUTS("server setup finished!");
  while (1) {
    DEBUG_PUTS("loop start");
    updateKqueue();
    clearEvlist(evlist);
    int nev = kevent(kq_, NULL, 0, evlist, kMaxEventSize, NULL);
    if (nev == 0)
      continue;
    else if (nev == -1)
      perror("kevent");
    for (int i = 0; i < nev; i++) {
      handleEvent(evlist[i]);
    }
    DEBUG_PUTS("----------------------");
  }
}
