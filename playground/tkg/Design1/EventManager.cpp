#include "EventManager.hpp"

#include "ConnectionSocket.hpp"
#include "ServerSocket.hpp"
#include "debug.hpp"

EventManager::EventManager() {
  kq_ = kqueue();
  if (kq_ == -1) {
    throw std::runtime_error("kqueue error");
  }
}

void EventManager::addServerSocket(int fd) { server_sockets_[fd] = new ServerSocket(fd); }

void EventManager::removeServerSocket(int fd) {
  delete server_sockets_[fd];
  server_sockets_.erase(fd);
}

void EventManager::addConnectionSocket(int fd) { connection_sockets_[fd] = new ConnectionSocket(fd); }

void EventManager::removeConnectionSocket(int fd) {
  delete connection_sockets_[fd];
  connection_sockets_.erase(fd);
}

void EventManager::addChangedEvents(struct kevent kevent) { changed_events_.push_back(kevent); }

void EventManager::registerServerEvent(int fd) {
  struct kevent chlist;
  bzero(&chlist, sizeof(struct kevent));
  DEBUG_PRINTF("server fd: %d, ", fd);
  EV_SET(&chlist, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
  DEBUG_PUTS("");
  kevent(kq_, &chlist, 1, NULL, 0, NULL);
  addServerSocket(fd);
}

void EventManager::updateKqueue() {
  int size = changed_events_.size();
  struct kevent chlist[size];
  bzero(chlist, sizeof(struct kevent) * size);
  int i = 0;
  for (changed_events_const_iterator itr = changed_events_.begin(); itr != changed_events_.end(); itr++, i++) {
    DEBUG_PRINTF("fd: %lu, ", (*itr).ident);
    EV_SET(&chlist[i], (*itr).ident, (*itr).filter, (*itr).flags, (*itr).fflags, (*itr).data, (*itr).udata);
  }
  DEBUG_PUTS("");
  kevent(kq_, chlist, size, NULL, 0, NULL);
  changed_events_.clear();
}

bool EventManager::isServerFd(int fd) { return server_sockets_.find(fd) != server_sockets_.end(); }

void EventManager::handleEvent(struct kevent ev) {
  if (ev.filter == EVFILT_TIMER) {
    DEBUG_PUTS("timeout");
    handleTimeout(ev);
  } else if (ev.filter == EVFILT_READ) {
    if (isServerFd(ev.ident)) {
      DEBUG_PUTS("request to make connection");
      server_sockets_[ev.ident]->make_client_connection(*this);
    } else {
      std::cout << "request connection fd " << std::endl;
      connection_sockets_[ev.ident]->handle_request(*this);
    }
  } else if (ev.filter == EVFILT_WRITE) {
    std::cout << "response connection fd " << std::endl;
    connection_sockets_[ev.ident]->handle_response(*this);
  }
}

void EventManager::handleTimeout(struct kevent ev) {
  close(ev.ident);
  removeConnectionSocket(ev.ident);
  addChangedEvents((struct kevent){ev.ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
}

void EventManager::clearEvlist(struct kevent *evlist) { bzero(evlist, sizeof(struct kevent) * kMaxEventSize); }

void EventManager::eventLoop() {  // confファイルを引数として渡す？

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
