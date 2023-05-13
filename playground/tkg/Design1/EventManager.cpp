#include "EventManager.hpp"

#include "ConnectionSocket.hpp"
#include "ServerSocket.hpp"

EventManager::EventManager() {}

void EventManager::addSocket(int fd, SockType type) {
  if (type == kTypeServer)
    sockets_[fd] = new ServerSocket(fd);
  else if (type == kTypeConnection)
    sockets_[fd] = new ConnectionSocket(fd);
}

void EventManager::removeSocket(int fd) {
  delete sockets_[fd];
  sockets_.erase(fd);
}

// changed fds functions
std::map<int, SockInfo> &EventManager::getChangedFds() { return changed_fds_; }

void EventManager::addChangedFd(int fd, SockInfo info) { changed_fds_[fd] = info; }

// void EventManager::make_client_connection(int port_fd) {
//   struct sockaddr_in add;
//   int addlen;
//   int connection_fd = accept(port_fd, (struct sockaddr *)&add, (socklen_t *)&addlen);
//   if (connection_fd == -1) {
//     throw std::runtime_error("accept error");
//   }
//   addChangedFd(connection_fd, EV_ADD);
//   return;
// }

void EventManager::open_port() {
  int port_fd;
  struct sockaddr_in add;

  if ((port_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    throw std::runtime_error("socket error");
  }
  add.sin_family = AF_INET;
  add.sin_addr.s_addr = INADDR_ANY;
  add.sin_port = htons(PORT);

  if (bind(port_fd, (struct sockaddr *)&add, sizeof(add)) == -1) {
    throw std::runtime_error("bind error");
  }
  if (listen(port_fd, 0) < 0) {
    throw std::runtime_error("listen error");
  }
  SockInfo info = {.type = kTypeServer, .flags = EV_ADD};
  addChangedFd(port_fd, info);
  return;
}

void EventManager::update_chlist(int kq) {
  int size = changed_fds_.size();
  struct kevent chlist[size];
  bzero(chlist, sizeof(struct kevent) * size);
  int i = 0;
  for (const_map_iterator itr = changed_fds_.begin(); itr != changed_fds_.end(); itr++, i++) {
    DEBUG_PRINTF("fd: %d, ", itr->first);
    SockInfo info = itr->second;
    EV_SET(&chlist[i], itr->first, EVFILT_READ, info.flags, 0, 0, 0);
    if (info.flags & EV_ADD) addSocket(itr->first, info.type);
  }
  DEBUG_PUTS("");
  changed_fds_.clear();
  kevent(kq, &chlist[0], size, NULL, 0, NULL);
}

void EventManager::eventLoop() {  // confファイルを引数として渡す？
  int kq = kqueue();
  if (kq == -1) {
    throw std::runtime_error("kqueue error");
  }
  open_port();
  struct kevent evlist[kEventSize];
  DEBUG_PUTS("server setup finished!");
  while (1) {
    DEBUG_PUTS("loop start");
    update_chlist(kq);
    bzero(evlist, sizeof(struct kevent) * kEventSize);
    int nev = kevent(kq, NULL, 0, evlist, kEventSize, NULL);
    if (nev == 0)
      continue;
    else if (nev == -1)
      perror("kevent");
    for (int i = 0; i < nev; i++) {
      sockets_[evlist[i].ident]->notify(*this);
    }
    DEBUG_PUTS("----------------------");
  }
}