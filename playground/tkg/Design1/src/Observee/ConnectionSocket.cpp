#include "ConnectionSocket.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>

#include "CGI.hpp"
#include "GET.hpp"

void ConnectionSocket::shutdown() {
  DEBUG_PUTS("ConnectionSocket shutdown");
  close(id_);
  em_->addChangedEvents((struct kevent){static_cast<uintptr_t>(id_), EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

GET *ConnectionSocket::makeGET(int fd) {
  GET *obs = new GET(fd, em_, this, &response_);
  this->monitorChild(obs);
  return obs;
}

CGI *ConnectionSocket::makeCGI(int id, int pid) {
  CGI *obs = new CGI(id, pid, em_, this, &response_);
  this->monitorChild(obs);
  return obs;
}

void ConnectionSocket::execCGI(const ServerConf *serv_conf) {
  // todo: ここでのgetLocationConfはMethod,cgi拡張子を引数として渡し、全てを満たすlocationConを返すようにしたい
  const LocationConf &loc_conf = serv_conf->getLocationConf(request_.getRequestTarget().absolute_path);
  std::string path = "." + loc_conf.getTargetPath(request_.getRequestTarget().absolute_path);
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
    // todo: close all observee fds
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
    em_->add(std::pair<t_id, t_type>(need_fd, FD), obs);
    em_->disableReadAndAddReadEvent(id_, need_fd);
  }
}

void ConnectionSocket::processGET(const ServerConf *serv_conf) {
  // todo: getLocationConfはMethodを引数として渡し、そのmethodが許可されているlocationConを返すようにしたい
  const LocationConf &loc_conf = serv_conf->getLocationConf(request_.getRequestTarget().absolute_path);
  std::string path = "." + loc_conf.getTargetPath(request_.getRequestTarget().absolute_path);
  // check directory or file exists
  struct stat st;
  if (stat(path.c_str(), &st) == -1) {
    response_.setStatus(404);
    throw std::runtime_error("stat error");  // 404 Not Found
  }
  // check directory or file is readable
  if (access(path.c_str(), R_OK) != 0) {
    response_.setStatus(403);
    throw std::runtime_error("access error");  // 403 Forbiden
  }
  bool is_directory = (st.st_mode & S_IFMT) == S_IFDIR;
  if (is_directory) {
    // see through index files (if no index files and autoindex is on, you should create directory list)
    std::string idx_path;
    if (loc_conf.common_.index_.size() != 0) {
      idx_path = loc_conf.common_.getIndexFile(path);
      if (idx_path != "") path = idx_path;
    }
    if (idx_path == "" && loc_conf.common_.autoindex_) {
      try {
        response_.appendBody(GET::listFilesAndDirectories(path));
      } catch (std::runtime_error &e) {
        // todo: read error is 5xx error?
        response_.setStatus(403);
        throw std::runtime_error(e);
      }
      response_.setStatus(200);
      DEBUG_PUTS("autoindex");
      em_->disableReadAndAddWriteEvent(id_, id_);
      return;  // 200 OK
    }
    if (idx_path == "" && !loc_conf.common_.autoindex_) {
      response_.setStatus(404);
      throw std::runtime_error("no index and autoindex");  // 404 Not Found
    }
  }
  int fd = open(path.c_str(), O_RDONLY);
  GET *obs = makeGET(fd);
  em_->add(std::pair<t_id, t_type>(fd, FD), obs);
  em_->disableReadAndAddReadEvent(id_, fd);
}

void ConnectionSocket::process() {
  const ServerConf *serv_conf = conf_.getServerConf(port_, request_.getHost().uri_host);

  // todo: 他の拡張子も対応できるようにする
  if (request_.getRequestTarget().absolute_path.find(".cgi") != std::string::npos) {
    try {
      execCGI(serv_conf);
    } catch (std::runtime_error &e) {
      std::cerr << e.what() << std::endl;
      em_->disableReadAndAddWriteEvent(id_, id_);
    }
  } else if (request_.methodIs(HttpRequest::GET)) {
    // handle GET
    try {
      processGET(serv_conf);
    } catch (std::runtime_error &e) {
      std::cerr << e.what() << std::endl;
      em_->disableReadAndAddWriteEvent(id_, id_);
    }
  } else if (request_.methodIs(HttpRequest::POST)) {
    // handle POST
  } else if (request_.methodIs(HttpRequest::DELETE)) {
    // handle DELETE
  }
  DEBUG_PUTS("PROCESSING FINISHED");
}

void ConnectionSocket::notify(struct kevent ev) {
  DEBUG_PUTS("ConnectionSocket notify");
  if (ev.filter == EVFILT_READ) {
    DEBUG_PUTS("handle_request() called");
    try {
      bool finished_reading = HttpRequest::readRequest(request_, *em_, rc_);
      if (finished_reading) {
        this->process();
      }
      // todo(thara): handle exceptions
      // } catch (const HttpRequest::BadRequestException &e) {
      // } catch (const HttpRequest::NotImplementedException &e) {
      // } catch (const HttpRequest::NotAllowedException &e) {
      // } catch (const HttpRequest::VersionNotSupportedException &e) {
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      close(ev.ident);
      em_->remove(std::pair<t_id, t_type>(ev.ident, FD));
      em_->addChangedEvents((struct kevent){static_cast<uintptr_t>(ev.ident), EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
    } catch (std::runtime_error &e) {
      std::cout << e.what() << std::endl;
      close(ev.ident);
      em_->remove(std::pair<t_id, t_type>(ev.ident, FD));
      em_->addChangedEvents((struct kevent){static_cast<uintptr_t>(ev.ident), EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
    }
  }
  if (ev.filter == EVFILT_WRITE) {
    DEBUG_PUTS("handle_response() called");
    request_.refresh();
    response_.createResponse();
    response_.sendResponse(*em_);
  }
}
