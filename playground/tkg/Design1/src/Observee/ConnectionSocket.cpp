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
#include <sstream>
#include <stdexcept>

#include "../Config/validation.h"
#include "../HttpException.hpp"
#include "CGI.hpp"
#include "GET.hpp"
#include "helper.hpp"

void ConnectionSocket::shutdown() {
  DEBUG_PUTS("ConnectionSocket shutdown");
  close(id_);
  em_->deleteTimerEvent(id_);
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

void ConnectionSocket::execCGI(const std::string &path) {
  // todo: .extensionから?#までの間をPATH_INFOとして分離する必要がある
  DEBUG_PUTS("MAKE EXEC");
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
    em_->disableReadEvent(id_);
    em_->registerReadEvent(need_fd);
  }
}

void ConnectionSocket::processGET() {
  if (!isAcceptableMethod(loc_conf_, HttpRequest::GET)) {
    throw BadRequestException("No Suitable Location");
  }
  // todo: . is for temporary implementation
  std::string path = "." + loc_conf_->getTargetPath(request_.getRequestTarget().absolute_path);
  // check directory or file exists
  struct stat st;
  if (stat(path.c_str(), &st) == -1) {
    throw ResourceNotFoundException("stat error: file doesn't exist");  // 404 Not Found
  }
  bool is_directory = (st.st_mode & S_IFMT) == S_IFDIR;
  // if CGI extension exist and that is not directory, try exec CGI
  const bool hasCGI = extension_ != "";
  if (hasCGI && !is_directory) {
    if (isExecutable(path.c_str()) && contain(loc_conf_->cgi_exts_, extension_)) {
      execCGI(path);
      return;
    }
  }
  // check directory or file is readable
  if (!isReadable(path.c_str())) {
    throw ResourceForbidenException("access Read error");  // 403 Forbiden
  }
  if (is_directory) {
    // see through index files (if no index files and autoindex is on, you should create directory list)
    std::string idx_path;
    if (loc_conf_->common_.index_.size() != 0) {
      idx_path = loc_conf_->common_.getIndexFile(path);
      if (idx_path != "") path = idx_path;
    }
    if (idx_path == "" && loc_conf_->common_.autoindex_) {
      try {
        response_.appendBody(GET::listFilesAndDirectories(path));
      } catch (HttpException &e) {
        throw e;
      }
      DEBUG_PUTS("autoindex");
      response_.setStatus(200);
      em_->disableReadEvent(id_);
      em_->registerWriteEvent(id_);
      return;  // 200 OK
    }
    if (idx_path == "" && !loc_conf_->common_.autoindex_) {
      throw ResourceNotFoundException("no index and autoindex");  // 404 Not Found
    }
  }
  // URI file or index file
  int fd = open(path.c_str(), O_RDONLY);
  GET *obs = makeGET(fd);
  em_->add(std::pair<t_id, t_type>(fd, FD), obs);
  em_->disableReadEvent(id_);
  em_->registerReadEvent(fd);
}

void ConnectionSocket::process() {
  ServerConf *serv_conf = conf_.getServerConf(port_, request_.getHost().uri_host);
  loc_conf_ = serv_conf->getLocationConf(&request_);
  // loc_conf is redirection block
  if (loc_conf_->hasRedirectDirective()) {
    response_.setStatus(std::atoi(loc_conf_->getRedirectStatus().c_str()));
    response_.appendHeader("Location", loc_conf_->getRedirectURI());
    em_->disableReadEvent(id_);
    em_->registerWriteEvent(id_);
    return;
  }
  extension_ = getExtension(request_.getRequestTarget().absolute_path);
  if (request_.methodIs(HttpRequest::GET)) {
    // handle GET
    try {
      processGET();
    } catch (HttpException &e) {
      throw e;
    }
  } else if (request_.methodIs(HttpRequest::POST)) {
    // handle POST
  } else if (request_.methodIs(HttpRequest::DELETE)) {
    // handle DELETE
  }
  DEBUG_PUTS("PROCESSING FINISHED");
}

void ConnectionSocket::processErrorPage(LocationConf *conf) {
  std::stringstream ss;
  ss << response_.getStatus();
  std::map<std::string, std::string>::const_iterator itr = conf->common_.error_pages_.find(ss.str());
  if (itr != conf->common_.error_pages_.end()) {
    std::string filename = itr->second;
    if (filename[0] != '/') filename = conf->common_.root_ + "/" + filename;
    ss << filename;
    std::string *error_page = conf_.cache_.status_errorPage_map_[ss.str()];
    if (error_page) response_.appendBody(*error_page);
  }
}

void ConnectionSocket::notify(struct kevent ev) {
  DEBUG_PUTS("ConnectionSocket notify");
  if (ev.filter == EVFILT_READ) {
    DEBUG_PUTS("handle_request() called");
    try {
      HttpRequest::State state = HttpRequest::readRequest(request_, rc_);
      if (state == HttpRequest::FinishedReading) {
        DEBUG_PRINTF("FINISHED READING: %s \n", escape(request_.getBody()).c_str());

        this->process();
      } else if (state == HttpRequest::SocketClosed) {
        shutdown();
      }
    } catch (HttpException &e) {
      // all error(readRequest and process) is handled here
      response_.setStatus(e.statusCode());
      if (loc_conf_) {
        // error_page directive is ignored when bad request
        processErrorPage(loc_conf_);
        // todo: redirect handle need to be done here?
      }
      em_->disableReadEvent(id_);
      em_->registerWriteEvent(id_);
    }
  }
  if (ev.filter == EVFILT_WRITE) {
    DEBUG_PUTS("handle_response() called");
    response_.createResponse();
    if (response_.sendResponse(*em_)) {
      loc_conf_ = NULL;
      request_ = HttpRequest(id_, &conf_);
      response_ = HttpResponse(id_, port_, &conf_);
      em_->disableWriteEvent(id_);
      em_->registerReadEvent(id_);
    }
  }
}
