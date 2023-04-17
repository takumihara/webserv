#ifndef EVENT_MANAGER_HPP_
#define EVENT_MANAGER_HPP_

#include <set>
#include <stdexcept>

class EventManager {
 public:
  typedef std::set<int>::iterator iterator;
  typedef std::set<int>::const_iterator const_iterator;

  EventManager();
  std::set<int> &getPortFds();
  void addPortFd(int fd);
  void removePortFd(int fd);
  std::set<int> &getConnectionFds();
  void addConnectionFd(int fd);
  void removeConnectionFd(int fd);
  std::set<int> &getNewFds();
  void addNewFd(int fd);
  void removeNewFd(int fd);
  void make_client_connection(int port_fd);
  void open_port();
  void eventLoop();
  void update_chlist(std::vector<struct kevent> &chlist);
  void send_response(int socket_fd, char *response);
  void handle_request(std::set<int> &socks, int fd);//std::vector<struct kevent> &chlist, int socket_fd);

 private:
  std::set<int> port_fds_;
  std::set<int> connection_fds_;
  size_t num_sockets_;
};

#endif