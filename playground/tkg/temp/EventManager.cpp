#include "EventManager.hpp"

EventManager::EventManager():num_sockets_(0) {}

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


#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <set>


#define PORT 80
#define max(x, y) ((x) > (y) ? (x) : (y))

// listenのqueueのsizeを0にしても2個目のクライアントがconnectできたのなぜ？
// -> 同時に接続リクエストが来た時。connectが完了したら、queueからは消える。


void EventManager::make_client_connection(int port_fd) {
  struct sockaddr_in add;
  int addlen;
  int connection_fd = accept(port_fd, (struct sockaddr *)&add, (socklen_t *)&addlen);
  if (connection_fd == -1) {
    throw std::runtime_error("accept error");
  }
  addConnectionFd(connection_fd);
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

void EventManager::handle_request(std::set<int> &socks, int socket_fd){// std::vector<struct kevent> &chlist, int socket_fd) {
  char request[100];
  int size;
  bzero(request, 100);
  if ((size = read(socket_fd, request, 100)) == -1) {
    return;//throw std::runtime_error("read error");
  }
  if (size == 0) {
    std::cout << "closed fd = " << socket_fd << std::endl;
    //EV_SET(&chlist[1], socket_fd, EVFILT_READ, EV_ADD, 0, 0, 0);
    close(socket_fd);
    socks.erase(socket_fd);
  } else {
    std::cout << "request received"
              << "(fd:" << socket_fd << "): '" << request << "'" << std::endl;
    send_response(socket_fd, request);
  }
 // (void)chlist;
}

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
  addPortFd(port_fd);
  return;
}

void  EventManager::update_chlist(std::vector<struct kevent> &chlist) {
  num_sockets_ = port_fds_.size() + connection_fds_.size();
  if (chlist.capacity() < num_sockets_){
   chlist.resize(num_sockets_);
  }
  // EV_SET(&chlist[0], 10, EVFILT_READ, EV_ADD, 0, 0, 0);
  // EV_SET(&chlist[0], 10, EVFILT_READ, EV_DISABLE, 0, 0, 0);
  std::cout << "port fds" << std::endl;
  int i = 0;
  for (const_iterator itr = port_fds_.begin(); itr != port_fds_.end(); itr++, i++) {
    std::cout << *itr << std::endl;
    EV_SET(&chlist[i], *itr, EVFILT_READ, EV_ADD, 0, 0, 0);
  }
  std::cout << "connection fds" << std::endl;
  for (const_iterator itr = connection_fds_.begin(); itr != connection_fds_.end(); itr++, i++) {
    std::cout << *itr << std::endl;
    EV_SET(&chlist[i], *itr, EVFILT_READ, EV_ADD, 0, 0, 0);
  }
}


void EventManager::eventLoop() { //confファイルを引数として渡す？
  open_port();
  int kq = kqueue();
  if (kq == -1) {
    throw std::runtime_error("kqueue error");
  }
  std::vector<struct kevent> chlist;
  std::vector<struct kevent> evlist;
  std::cout << "server setup finished!" << std::endl;
  while (1) {
    std::cout << "loop start" << std::endl;
    update_chlist(chlist);
    evlist.resize(chlist.size());
    int nev = kevent(kq, &(*chlist.begin()), chlist.size(), &(*evlist.begin()), evlist.size(), NULL);
    if (nev == 0)
      continue;
    else if (nev == -1)
      perror("kevent");
    for (const_iterator itr = connection_fds_.begin(); itr != connection_fds_.begin(); itr++) {
      std::cout << *itr << std::endl;
    }
    for (int i = 0; i < nev; i++) {
      if (port_fds_.find(evlist[i].ident) != port_fds_.end()) {
        make_client_connection(evlist[i].ident);
      } else {
        //handle_request(connection_fds_, chlist, evlist[i].ident);
        handle_request(connection_fds_, evlist[i].ident);
      }
    }
    std::cout << "----------------------" << std::endl;
  }
}
