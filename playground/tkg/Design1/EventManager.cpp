#include "EventManager.hpp"

#include "ConnectionSocket.hpp"
#include "ServerSocket.hpp"
#include "debug.hpp"

EventManager::EventManager() {}

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

// void EventManager::addSocket(int fd, SockType type) {
//   if (type == kTypeServer)
//     sockets_[fd] = new ServerSocket(fd);
//   else if (type == kTypeConnection)
//     sockets_[fd] = new ConnectionSocket(fd);
// }

// void EventManager::removeSocket(int fd) {
//   delete sockets_[fd];
//   sockets_.erase(fd);
// }

std::map<int, SockInfo> &EventManager::getChangedFds() { return changed_fds_; }

void EventManager::addChangedFd(int fd, SockInfo info) { changed_fds_[fd] = info; }

int EventManager::open_port() {
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
  addServerSocket(port_fd);
  return port_fd;
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
    if (info.type == kTypeConnection && info.flags & EV_ADD) addConnectionSocket(itr->first);
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
  int port_fd = open_port();
  (void)port_fd;
  struct kevent evlist[kMaxEventSize];
  DEBUG_PUTS("server setup finished!");
  while (1) {
    DEBUG_PUTS("loop start");
    update_chlist(kq);
    bzero(evlist, sizeof(struct kevent) * kMaxEventSize);
    int nev = kevent(kq, NULL, 0, evlist, kMaxEventSize, NULL);
    if (nev == 0)
      continue;
    else if (nev == -1)
      perror("kevent");
    for (int i = 0; i < nev; i++) {
      if (server_sockets_.find(evlist[i].ident) != server_sockets_.end()) {
        DEBUG_PUTS("port fd ");
        server_sockets_[evlist[i].ident]->make_client_connection(*this);
      } else {
        std::cout << "connection fd " << std::endl;
        connection_sockets_[evlist[i].ident]->handle_request(*this);
      }
    }
    DEBUG_PUTS("----------------------");
  }
}