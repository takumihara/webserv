#include "ConnectionSocket.hpp"

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

#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>

#include "CGI.hpp"

static std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("conf file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

void ConnectionSocket::shutdown(EventManager &em) {
  DEBUG_PUTS("ConnectionSocket shutdown");
  close(id_);
  em.addChangedEvents((struct kevent){id_, EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
  em.remove(std::pair<t_id, t_type>(id_, FD));
}

CGI *ConnectionSocket::makeCGI(int id, int pid) {
  CGI *obs = new CGI(id, pid, this, &result_);
  this->monitorChild(obs);
  return obs;
}

void ConnectionSocket::execCGI(const std::string &path, EventManager &event_manager) {
  std::cout << "MAKE EXEC\n";
  extern char **environ;
  char *argv[2];
  argv[0] = const_cast<char *>(path.c_str());
  argv[1] = NULL;
  int fd[2];
  int need_fd;

  pipe(fd);
  need_fd = fd[0];
  int pid = fork();
  if (pid == 0) {
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    if (execve(path.c_str(), argv, environ) == -1) {
      perror("execve");
      close(fd[1]);
      exit(1);
    }
  } else {
    close(fd[1]);
    std::cout << "pid: " << pid << std::endl;
    CGI *obs = makeCGI(need_fd, pid);
    std::cout << "need_fd: " << need_fd << "  pid: " << pid << std::endl;
    event_manager.add(std::pair<t_id, t_type>(need_fd, FD), obs);
    event_manager.addChangedEvents((struct kevent){id_, EVFILT_READ, EV_DISABLE, 0, 0, 0});
    event_manager.addChangedEvents((struct kevent){need_fd, EVFILT_READ, EV_ADD, 0, 0, 0});
    event_manager.addChangedEvents(
        (struct kevent){need_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
  }
}

void ConnectionSocket::process(EventManager &event_manager) {
  const ServerConf *serv_conf = conf_.getServerConf(port_, request_.getHost().uri_host);
  const LocationConf &loc_conf = conf_.getLocationConf(serv_conf, request_.getRequestTarget().absolute_path);

  // todo: check if file exists
  const std::string path = "." + loc_conf.common_.root_ + request_.getRequestTarget().absolute_path;
  DEBUG_PRINTF("path: %s\n", path.c_str());
  std::string content;
  if (path.find(".cgi") != std::string::npos) {
    execCGI(path, event_manager);
  } else {
    event_manager.addChangedEvents((struct kevent){id_, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0});
    event_manager.addChangedEvents(
        (struct kevent){id_, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
    result_ = readFile(path.c_str());
  }
  DEBUG_PUTS("PROCESSING FINISHED");
}

void ConnectionSocket::notify(EventManager &event_manager, struct kevent ev) {
  DEBUG_PUTS("ConnectionSocket notify");
  if (ev.filter == EVFILT_READ) {
    DEBUG_PUTS("handle_request() called");
    bool finished = request_.readRequest(event_manager);
    if (finished) {
      this->process(event_manager);
    }
  }
  if (ev.filter == EVFILT_WRITE) {
    DEBUG_PUTS("handle_response() called");
    request_.refresh();
    response_.createResponse(result_);
    response_.sendResponse(event_manager);
    // request_.headers_ .clear();
  }
}