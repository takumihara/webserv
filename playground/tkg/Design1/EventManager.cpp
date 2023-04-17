#include "EventManager.hpp"

EventManager::EventManager() {}

//Port fds functions
std::set<int> &EventManager::getPortFds() {
  return port_fds_;
}

void	EventManager::addPortFd(int fd) {
	port_fds_.insert(fd);
}

void	EventManager::removePortFd(int fd) {
	port_fds_.erase(fd);
}

//client connection fds functions
std::set<int> &EventManager::getConnectionFds() {
  return connection_fds_;
}

void	EventManager::addConnectionFd(int fd) {
	connection_fds_.insert(fd);
}

void	EventManager::removeConnectionFd(int fd) {
	connection_fds_.erase(fd);
}

//changed fds functions
std::map<int, int> &EventManager::getChangedFds() {
  return changed_fds_;
}

void	EventManager::addChangedFd(int fd, int flag) {
	changed_fds_[fd] = flag;
}


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

void EventManager::send_response(int socket_fd, char *response) {
  int res = sendto(socket_fd, response, strlen(response), 0, NULL, 0);
  if (res == -1) {
    throw std::runtime_error("send error");
  }
  std::cout << "response sent: "
            << "'" << response << "'"
            << " (" << res << ")" << std::endl;
}

void EventManager::handle_request(int socket_fd) {
  char request[100];
  int size;
  bzero(request, 100);
  if ((size = read(socket_fd, request, 100)) == -1) {
    throw std::runtime_error("read error");
  }
  if (size == 0) {
    std::cout << "closed fd = " << socket_fd << std::endl;
    removeConnectionFd(socket_fd);
    close(socket_fd);
  } else {
    std::cout << "request received"
              << "(fd:" << socket_fd << "): '" << request << "'" << std::endl;
    send_response(socket_fd, request);
  }
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
  addPortFd(port_fd);
  struct kevent pevent;
  EV_SET(&pevent, port_fd, EVFILT_READ, EV_ADD, 0, 0, 0);
  kevent(kq, &pevent, 1, NULL, 0, NULL);
  return;
}

void  EventManager::update_chlist(int kq, std::vector<struct kevent> &chlist) {

  chlist.clear();
  if (chlist.capacity() < changed_fds_.size())
    chlist.reserve(changed_fds_.size());
  int i = 0;
  for (const_map_iterator itr = changed_fds_.begin(); itr != changed_fds_.end(); itr++, i++) {
    std::cout << itr->first  << " " << itr->second  << " " << EV_ADD << std::endl;
    EV_SET(&chlist[i], itr->first, EVFILT_READ, itr->second, 0, 0, 0);
    if (itr->second & EV_ADD)
      addConnectionFd(itr->first);
  }
  changed_fds_.clear();
  kevent(kq, &(*chlist.begin()), num_events_, NULL, 0, NULL);
}

void EventManager::eventLoop() { //confファイルを引数として渡す？
  int kq = kqueue();
  if (kq == -1) {
    throw std::runtime_error("kqueue error");
  }
  open_port(kq);
  std::vector<struct kevent> chlist;
  chlist.reserve(100);
  std::vector<struct kevent> evlist;
  evlist.reserve(100);
  std::cout << "server setup finished!" << std::endl;
  while (1) {
    std::cout << "loop start" << std::endl;
    update_chlist(kq, chlist);
    evlist.clear();
    int nev = kevent(kq, NULL, 0, &(*evlist.begin()), 100, NULL);
    if (nev == 0)
      continue;
    else if (nev == -1)
      perror("kevent");
    for (int i = 0; i < nev; i++) {
      if (port_fds_.find(evlist[i].ident) != port_fds_.end()) {
        std::cout << "port fd " << std::endl;
        make_client_connection(evlist[i].ident);
      } else {
        std::cout << "connection fd " << std::endl;
        handle_request(evlist[i].ident);
      }
    }
    std::cout << "----------------------" << std::endl;
  }
}
