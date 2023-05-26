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

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "const.hpp"

ConnectionSocket::ConnectionSocket(int fd, int port, Config &conf)
    : sock_fd_(fd),
      port_(port),
      conf_(conf),
      result_(""),
      request_(HttpRequest(fd, port)),
      response_(HttpResponse(fd, port)) {}

void ConnectionSocket::handle_response(EventManager &event_manager) {
  DEBUG_PUTS("handle_response() called");
  request_.refresh();
  response_.createResponse(result_);
  response_.sendResponse(event_manager);
}

static std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("conf file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

void ConnectionSocket::process(EventManager &event_manager) {
  const ServerConf *serv_conf = conf_.getServConfig(port_, request_.getHeaderValue("host"));
  const LocationConf &loc_conf = serv_conf->getLocationConfig(request_.getRequestTarget());

  // todo: check if file exists
  const std::string path = "." + loc_conf.common_.root_ + request_.getRequestTarget();
  DEBUG_PRINTF("path: %s\n", path.c_str());
  std::string content;
  if (path.find(".cgi") != std::string::npos) {
    execCgi(path, event_manager);
  } else {
    event_manager.addChangedEvents((struct kevent){sock_fd_, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0});
    event_manager.addChangedEvents(
        (struct kevent){sock_fd_, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
    result_ = readFile(path.c_str());
  }
  DEBUG_PUTS("PROCESSING FINISHED");
}

void ConnectionSocket::execCgi(const std::string &path, EventManager &event_manager) {
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
    event_manager.addCgiConnectionPair(need_fd, this);

    event_manager.addChangedEvents((struct kevent){need_fd, EVFILT_READ, EV_ADD, 0, 0, 0});
    event_manager.addChangedEvents(
        (struct kevent){need_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
  }
}

void ConnectionSocket::handleCGI(EventManager &event_manager, int cgi_fd) {
  std::cout << "handle CGI" << std::endl;
  char buff[SOCKET_READ_SIZE + 1];
  int res = read(cgi_fd, buff, FILE_READ_SIZE);
  buff[res] = '\0';
  result_ += buff;
  std::cout << "cgi wip result: '" << result_ << "'" << std::endl;

  if (res == 0) {
    close(cgi_fd);
    event_manager.removeCgiConnectionPair(cgi_fd);
    event_manager.addChangedEvents(
        (struct kevent){cgi_fd, EVFILT_TIMER, EV_DELETE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
    event_manager.addChangedEvents((struct kevent){sock_fd_, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0});
    event_manager.addChangedEvents(
        (struct kevent){sock_fd_, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
    std::cout << "CGI LAST RESULT: '" << result_ << "'" << std::endl;
  }
}

bool ConnectionSocket::handle_request(EventManager &event_manager) { return request_.readRequest(event_manager); }
