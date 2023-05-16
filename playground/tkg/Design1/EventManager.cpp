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

void EventManager::addChangedFd(int fd, SockInfo info) { changed_fds_[fd] = info; }

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
  int size = changed_fds_.size();
  struct kevent chlist[size];
  bzero(chlist, sizeof(struct kevent) * size);
  int i = 0;
  for (const_map_iterator itr = changed_fds_.begin(); itr != changed_fds_.end(); itr++, i++) {
    DEBUG_PRINTF("fd: %d, ", itr->first);
    SockInfo info = itr->second;
    EV_SET(&chlist[i], itr->first, EVFILT_READ, info.flags, 0, 0, 0);
  }
  DEBUG_PUTS("");
  kevent(kq_, &chlist[0], size, NULL, 0, NULL);
  changed_fds_.clear();
}

bool EventManager::isServerFd(int fd) { return server_sockets_.find(fd) != server_sockets_.end(); }

void EventManager::handleEvent(int fd) {
  if (isServerFd(fd)) {
    DEBUG_PUTS("port fd ");
    server_sockets_[fd]->make_client_connection(*this);
  } else {
    std::cout << "connection fd " << std::endl;
    connection_sockets_[fd]->handle_request(*this);
  }
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
      handleEvent(evlist[i].ident);
    }
    DEBUG_PUTS("----------------------");
  }
}
