#include "EventManager.hpp"

#include "ConnectionSocket.hpp"
#include "ServerSocket.hpp"

EventManager::EventManager() {}

// Port fds functions
// std::set<int> &EventManager::getPortFds() { return port_fds_; }

void EventManager::addServerSocket(int fd) { sockets_[fd] = new ServerSocket(fd); }

void EventManager::removeServerSocket(int fd) {
  delete sockets_[fd];
  sockets_.erase(fd);
}

// client connection fds functions
//  std::set<int> &EventManager::getConnectionFds() {
//    return connection_fds_;
//  }

void EventManager::addConnectionSocket(int fd) { sockets_[fd] = new ConnectionSocket(fd, ConnectionSocket::kSocFree); }

void EventManager::removeConnectionSocket(int fd) {
  delete sockets_[fd];
  sockets_.erase(fd);
}

// changed fds functions
std::map<int, int> &EventManager::getChangedFds() { return changed_fds_; }

void EventManager::addChangedFd(int fd, int flag) { changed_fds_[fd] = flag; }

void EventManager::make_client_connection(int port_fd) {
  struct sockaddr_in add;
  int addlen;
  int connection_fd = accept(port_fd, (struct sockaddr *)&add, (socklen_t *)&addlen);
  if (connection_fd == -1) {
    throw std::runtime_error("accept error");
  }
  addChangedFd(connection_fd, EV_ADD);
  return;
}

void EventManager::open_port(int kq) {
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
  addServerSocket(port_fd);
  struct kevent pevent;
  EV_SET(&pevent, port_fd, EVFILT_READ, EV_ADD, 0, 0, 0);
  kevent(kq, &pevent, 1, NULL, 0, NULL);
  return;
}

void EventManager::update_chlist(int kq) {
  int size = changed_fds_.size();
  struct kevent chlist[size];
  bzero(chlist, sizeof(struct kevent) * size);
  int i = 0;
  for (const_map_iterator itr = changed_fds_.begin(); itr != changed_fds_.end(); itr++, i++) {
    DEBUG_PRINTF("fd: %d, ", itr->first);
    EV_SET(&chlist[i], itr->first, EVFILT_READ, itr->second, 0, 0, 0);
    if (itr->second & EV_ADD) addConnectionSocket(itr->first);
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
  open_port(kq);
  struct kevent evlist[kEventSize];
  DEBUG_PUTS("server setup finished!");
  while (1) {
    DEBUG_PUTS("loop start");
    update_chlist(kq);
    bzero(evlist, sizeof(struct kevent) * kEventSize);
    int nev = kevent(kq, NULL, 0, evlist, 100, NULL);
    if (nev == 0)
      continue;
    else if (nev == -1)
      perror("kevent");
    for (int i = 0; i < nev; i++) {
      sockets_[evlist[i].ident]->notify(*this);
      // if (port_fds_.find(evlist[i].ident) != port_fds_.end()) {
      //   std::cout << "port fd " << std::endl;
      //   make_client_connection(evlist[i].ident);
      // } else {
      //   std::cout << "connection fd " << std::endl;
      // }
    }
    DEBUG_PUTS("----------------------");
  }
}