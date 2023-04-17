#ifndef EVENT_MANAGER_HPP_
#define EVENT_MANAGER_HPP_

#include <set>
#include <map>
#include <stdexcept>

class EventManager {
 public:
  typedef std::set<int>::iterator set_iterator;
  typedef std::set<int>::const_iterator const_set_iterator;
  typedef std::map<int, int>::iterator map_iterator;
  typedef std::map<int, int>::const_iterator const_map_iterator;

  EventManager();
  std::set<int> &getPortFds();
  void addPortFd(int fd);
  void removePortFd(int fd);
  std::set<int> &getConnectionFds();
  void addConnectionFd(int fd);
  void removeConnectionFd(int fd);
  std::map<int, int> &getChangedFds();
  void addChangedFd(int fd, int flag);
  void removeNewFd(int fd);
  void make_client_connection(int port_fd);
  void open_port(int kp);
  void eventLoop();
  void update_chlist(int kq, std::vector<struct kevent> &chlist);
  void update_evlist(std::vector<struct kevent> &evlist);
  void send_response(int socket_fd, char *response);
  void handle_request(int fd);
  struct s_eventInfo {
	bool read;
	bool write;
	bool except;
  };

 private:
  std::set<int> port_fds_;
  std::set<int> connection_fds_;
  std::map<int, int> changed_fds_;
  static int num_events_;
};

#endif