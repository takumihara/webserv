#include "EventManager.hpp"

#include <csignal>

#include "./Config/Config.hpp"
#include "Observee/Observee.hpp"
#include "debug.hpp"

EventManager::EventManager() {
  kq_ = kqueue();
  if (kq_ == -1) {
    throw std::runtime_error("kqueue error");
  }
}

void EventManager::addChangedEvents(struct kevent kevent) {
  t_key key(kevent.ident, kevent.filter);
  changed_events_[key] = kevent;
}

void EventManager::terminateAll() {
  for (std::map<std::pair<t_id, t_type>, Observee *>::iterator itr = observees_.begin(); itr != observees_.end();
       itr++) {
    (itr->second)->terminate();
    delete itr->second;
  }
  observees_.clear();
  close(kq_);
}

void EventManager::add(const std::pair<t_id, t_type> &key, Observee *obs) { observees_[key] = obs; }

void EventManager::remove(const std::pair<t_id, t_type> &key) {
  delete observees_[key];
  observees_.erase(key);
}

void EventManager::registerServerEvent(int fd, int port, Config &conf) {
  struct kevent chlist;
  bzero(&chlist, sizeof(struct kevent));
  DEBUG_PRINTF("server fd: %d, ", fd);
  EV_SET(&chlist, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
  DEBUG_PUTS("");
  kevent(kq_, &chlist, 1, NULL, 0, NULL);
  Observee *obs = new ServerSocket(fd, port, conf, this);
  std::cout << "sock fd: " << fd << std::endl;
  add(std::pair<t_id, t_type>(fd, FD), obs);
  observees_[std::pair<t_id, t_type>(fd, FD)];
  std::cout << observees_[std::pair<t_id, t_type>(fd, FD)]->id_;
}

void EventManager::registerWriteEvent(uintptr_t fd) {
  addChangedEvents((struct kevent){fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0});
  addChangedEvents((struct kevent){fd, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
}

void EventManager::registerReadEvent(uintptr_t fd) {
  addChangedEvents((struct kevent){fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0});
  addChangedEvents((struct kevent){fd, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
}

void EventManager::disableReadEvent(uintptr_t fd) {
  addChangedEvents((struct kevent){fd, EVFILT_READ, EV_DISABLE, 0, 0, 0});
}

void EventManager::disableWriteEvent(uintptr_t fd) {
  addChangedEvents((struct kevent){fd, EVFILT_WRITE, EV_DISABLE, 0, 0, 0});
}

void EventManager::deleteTimerEvent(uintptr_t fd) {
  addChangedEvents((struct kevent){fd, EVFILT_TIMER, EV_DELETE, 0, 0, 0});
}

void EventManager::disableTimerEvent(uintptr_t fd) {
  addChangedEvents((struct kevent){fd, EVFILT_TIMER, EV_DISABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
}

// enableTimerEvent() and updateTimer() do same thing but define two function for readability.
void EventManager::enableTimerEvent(uintptr_t fd) {
  addChangedEvents((struct kevent){fd, EVFILT_TIMER, EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
}

void EventManager::updateTimer(uintptr_t fd) {
  addChangedEvents((struct kevent){fd, EVFILT_TIMER, EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
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
  DEBUG_PUTS("---------Observees---------");
  for (std::map<t_key, Observee *>::iterator itr = observees_.begin(); itr != observees_.end(); itr++) {
    DEBUG_PRINTF("fd: %lu type: %hd\n", itr->first.first, itr->first.second);
  }
  DEBUG_PUTS("---------------------------");
  int size = changed_events_.size();
  struct kevent chlist[size];
  bzero(chlist, sizeof(struct kevent) * size);
  int i = 0;
  for (changed_events_const_iterator itr = changed_events_.begin(); itr != changed_events_.end(); itr++, i++) {
    const struct kevent &ev = itr->second;
    DEBUG_PRINTF("fd: %lu(%s:%s), ", ev.ident, getEventFilter(ev.filter).c_str(), getEventFlags(ev.flags).c_str());
    EV_SET(&chlist[i], ev.ident, ev.filter, ev.flags, ev.fflags, ev.data, ev.udata);
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
    observees_[std::pair<t_id, t_type>(ev.ident, getType(ev.filter))]->notify(ev);
  }
  DEBUG_PUTS("END HANDLE EVENT");
}

void EventManager::handleTimeout(struct kevent ev) { observees_[std::pair<t_id, t_type>(ev.ident, FD)]->timeout(); }

void EventManager::clearEvlist(struct kevent *evlist) { bzero(evlist, sizeof(struct kevent) * kMaxEventSize); }

void EventManager::eventLoop() {
  struct kevent evlist[kMaxEventSize];
  extern sig_atomic_t sig_int;
  DEBUG_PUTS("server setup finished!");
  while (1) {
    DEBUG_PUTS("loop start");
    updateKqueue();
    clearEvlist(evlist);
    int nev = kevent(kq_, NULL, 0, evlist, kMaxEventSize, NULL);
    if (sig_int) {
      DEBUG_PUTS("sig INT");
      terminateAll();
      exit(0);
    }
    if (nev == 0)
      continue;
    else if (nev == -1)
      perror("kevent");
    for (int i = 0; i < nev; i++) {
      DEBUG_PRINTF("evlist fd: %lu(%s:%s), ", evlist[i].ident, getEventFilter(evlist[i].filter).c_str(),
                   getEventFlags(evlist[i].flags).c_str());
      handleEvent(evlist[i]);
    }
    DEBUG_PUTS("----------------------");
  }
}
