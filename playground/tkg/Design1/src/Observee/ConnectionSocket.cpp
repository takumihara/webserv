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
#include "CGIInfo.hpp"
#include "GET.hpp"
#include "POST.hpp"
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

POST *ConnectionSocket::makePOST(int fd) {
  POST *obs = new POST(fd, em_, this, &request_);
  this->monitorChild(obs);
  return obs;
}

CGI *ConnectionSocket::makeCGI(int id, int pid) {
  CGI *obs = new CGI(id, pid, em_, this, &request_, &response_);
  this->monitorChild(obs);
  return obs;
}

void ConnectionSocket::execCGI(const std::string &path) {
  // todo: .extensionから?#までの間をPATH_INFOとして分離する必要がある
  DEBUG_PUTS("MAKE EXEC");
  extern char **environ;
  char *argv[2];
  argv[1] = NULL;
  int fd[2];

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1) {
    throw std::runtime_error("soketpair error");
  }
  int pid = fork();
  if (pid == 0) {
    close(fd[0]);
    CGIInfo info = parseCGI(path, extension_, request_, loc_conf_);
    setCGIInfo(info);
    argv[0] = const_cast<char *>(info.script_name_.c_str());
    if (dup2(fd[1], STDIN_FILENO) == -1) {
      perror("dup2");
      exit(1);
    }
    if (dup2(fd[1], STDOUT_FILENO) == -1) {
      perror("dup2");
      exit(1);
    }
    close(fd[1]);
    if (execve(info.script_name_.c_str(), argv, environ) == -1) {
      perror("execve");
      // close(fd[1]);
      exit(1);
    }
  } else {
    close(fd[1]);
    std::cout << "pid: " << pid << std::endl;
    CGI *obs = makeCGI(fd[0], pid);
    std::cout << "need_fd: " << fd[0] << "  pid: " << pid << std::endl;
    em_->add(std::pair<t_id, t_type>(fd[0], FD), obs);
    em_->disableReadEvent(id_);
    em_->registerWriteEvent(fd[0]);
  }
}

void ConnectionSocket::processPOST() {
  // todo:
  if (!isAcceptableMethod(loc_conf_, HttpRequest::POST)) {
    throw BadRequestException("No Suitable Location");
  }
  // todo: . is for temporary implementation
  std::string path = "." + loc_conf_->getTargetPath(request_.getRequestTarget()->getPath());
  // check file is already exist or not, and temporarily set statusCode.
  struct stat st;
  if (!isAllDirectoryWritable(path)) throw ResourceForbidenException("access Write error");  // 403 Forbiden
  const bool file_exist = stat(path.c_str(), &st) != -1;
  if (file_exist) {
    response_.setStatus(200);
    bool is_directory = (st.st_mode & S_IFMT) == S_IFDIR;
    // POST method is not allowedif targetURI is directory
    if (is_directory) throw MethodNotAllowedException("POST is not allowed to directory");
    // check writable if file exist
    if (!isWritable(path.c_str())) throw ResourceForbidenException("acess write error");
  }
  // if CGI extension exist, try exec CGI
  const bool hasCGI = extension_ != "";
  if (hasCGI) {
    if (isExecutable(path.c_str()) && contain(loc_conf_->cgi_exts_, extension_)) {
      execCGI(path);
      return;
    }
  }
  // set temporary status code
  if (file_exist)
    response_.setStatus(200);
  else
    response_.setStatus(201);
  int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
  if (fd < 0) throw InternalServerErrorException("open error");
  POST *obs = makePOST(fd);
  em_->add(std::pair<t_id, t_type>(fd, FD), obs);
  em_->disableReadEvent(id_);
  em_->registerWriteEvent(fd);
  return;
}

void ConnectionSocket::processGET() {
  if (!isAcceptableMethod(loc_conf_, HttpRequest::GET)) {
    throw BadRequestException("No Suitable Location");
  }
  // todo: . is for temporary implementation
  std::string path = "." + loc_conf_->getTargetPath(request_.getRequestTarget()->getPath());
  // if CGI extension exist, try exec CGI
  const bool hasCGI = extension_ != "";
  if (hasCGI) {
    if (contain(loc_conf_->cgi_exts_, extension_)) {
      execCGI(path);
      return;
    }
  }
  struct stat st;
  if (stat(path.c_str(), &st) == -1) {
    throw ResourceNotFoundException("stat error: file doesn't exist");  // 404 Not Found
  }
  bool is_directory = (st.st_mode & S_IFMT) == S_IFDIR;
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
      DEBUG_PUTS("autoindex");
      response_.appendBody(GET::listFilesAndDirectories(path));
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

void ConnectionSocket::processErrorPage(const LocationConf *conf) {
  std::stringstream ss;
  ss << response_.getStatus();
  std::map<std::string, std::string>::const_iterator itr = conf->common_.error_pages_.find(ss.str());
  if (itr != conf->common_.error_pages_.end()) {
    std::string filename = itr->second;
    if (filename[0] != '/') filename = conf->common_.root_ + "/" + filename;
    if (conf_.cache_.error_page_paths_.find(filename) != conf_.cache_.error_page_paths_.end())
      response_.appendBody(conf_.cache_.error_page_paths_[filename]);
  }
}

void ConnectionSocket::process() {
  ServerConf *serv_conf = conf_.getServerConf(port_, request_.getHost().uri_host);
  loc_conf_ = serv_conf->getLocationConf(&request_);
  // loc_conf is redirection block
  if (loc_conf_->hasRedirectDirective()) {
    response_.setStatus(std::atoi(loc_conf_->getRedirectStatus().c_str()));
    response_.appendHeader("Location", loc_conf_->getRedirectURI());
    throw RedirectMovedPermanently("redirection");
  }
  extension_ = getExtension(request_.getRequestTarget()->getPath());
  if (request_.methodIs(HttpRequest::GET)) {
    processGET();
  } else if (request_.methodIs(HttpRequest::POST)) {
    // handle POST
    processPOST();
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
      HttpRequestReader::State state = rreader_.readRequest();
      if (state == HttpRequestReader::FinishedReading) {
        DEBUG_PRINTF("FINISHED READING: %s \n", escape(request_.getBody()).c_str());
        this->process();
      }
    } catch (HttpException &e) {
      // all 3xx 4xx 5xx exception(readRequest and process) is catched here
      std::cerr << e.what();
      response_.setStatus(e.statusCode());
      if (loc_conf_) {
        // error_page directive is ignored when bad request
        processErrorPage(loc_conf_);
      }
      em_->disableReadEvent(id_);
      em_->registerWriteEvent(id_);
    } catch (std::runtime_error &e) {
      shutdown();
    }
  }
  if (ev.filter == EVFILT_WRITE) {
    DEBUG_PUTS("handle_response() called");
    response_.createResponse();
    response_.sendResponse();
    if (response_.getState() == HttpResponse::End) {
      loc_conf_ = NULL;
      request_ = HttpRequest();
      response_ = HttpResponse(id_, port_, &conf_);
      em_->disableWriteEvent(id_);
      em_->registerReadEvent(id_);
    }
  }
}
