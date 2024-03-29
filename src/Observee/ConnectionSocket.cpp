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
#include <string>

#include "../Config/validation.h"
#include "../HTML.hpp"
#include "../HttpException.hpp"
#include "CGI/CGI.hpp"
#include "CGI/CGIInfo.hpp"
#include "GET.hpp"
#include "helper.hpp"

void ConnectionSocket::timeout() {
  DEBUG_PUTS("ConnectionSocket::timeout");
  for (size_t i = 0; i < children_.size(); i++) {
    children_[i]->shutdown();
  }
  children_.clear();
  close(id_);
  em_->deleteTimerEvent(id_);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void ConnectionSocket::shutdown() {
  DEBUG_PUTS("ConnectionSocket shutdown");
  // for (std::vector<Observee *>::iterator itr = children_.begin(); itr != children_.end(); itr++) {
  //   (*itr)->parent_ = NULL;
  //   (*itr)->shutdown();
  // }
  // children_.clear();
  close(id_);
  em_->deleteTimerEvent(id_);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void ConnectionSocket::terminate() { close(id_); }

void ConnectionSocket::initExtension() { CGIextension_ = ""; }

HttpResponse *ConnectionSocket::initResponse() {
  response_ = HttpResponse(id_, port_, &conf_);
  return &response_;
}

GET *ConnectionSocket::makeGET(int fd) {
  DEBUG_PRINTF("ConnectionSocket::makeGET fd: %d\n", fd);
  GET *obs = new GET(fd, em_, this, &response_);
  this->monitorChild(obs);
  return obs;
}

CGI *ConnectionSocket::makeCGI(int id, int pid) {
  DEBUG_PRINTF("MAKE CGI fd: %d\n", id);
  CGI *obs = new CGI(id, pid, em_, this, &request_, &response_);
  this->monitorChild(obs);
  return obs;
}

void ConnectionSocket::execCGI(const std::string &path) {
  DEBUG_PUTS("EXEC CGI");
  char *argv[2];
  argv[1] = NULL;
  int fd[2];
  CGIInfo info = parseCGIInfo(path, CGIextension_, request_, loc_conf_);
  struct stat st;
  const bool file_exist = stat(info.script_name_.c_str(), &st) != -1;
  if (!file_exist) throw ResourceNotFoundException("cgi script is not found");
  if ((st.st_mode & S_IFMT) == S_IFDIR || !isExecutable(info.script_name_.c_str()))
    throw ResourceForbiddenException("cgi script name is directory or not excutable");
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1) {
    throw InternalServerErrorException("socketpair error");
  }
  if (fcntl(fd[0], F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1) {
    close(fd[0]);
    close(fd[1]);
    throw InternalServerErrorException("fcntl error");
  }
  int set = 1;
  if (setsockopt(fd[0], SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(set)) == -1) {
    close(fd[0]);
    close(fd[1]);
    throw InternalServerErrorException("setsockopt error");
  }
  int pid = fork();
  if (pid == 0) {
    close(fd[0]);
    em_->terminateAll();
    std::vector<char *> env;
    info.setEnv(env);
    std::string cwd = info.getCGIWorkingDirectory();
    if (chdir(cwd.c_str()) == -1) {
      perror("chdir");
      close(fd[1]);
      exit(1);
    }
    argv[0] = const_cast<char *>(info.script_name_.c_str());
    if (dup2(fd[1], STDIN_FILENO) == -1) {
      perror("dup2");
      close(fd[1]);
      exit(1);
    }
    if (dup2(fd[1], STDOUT_FILENO) == -1) {
      perror("dup2");
      close(fd[1]);
      exit(1);
    }
    close(fd[1]);
    if (execve(info.script_name_.c_str(), argv, &env[0]) == -1) {
      perror("execve");
      exit(1);
    }
    exit(1);
  } else {
    close(fd[1]);
    DEBUG_PRINTF("pid: %d, cgi fd: %d\n", pid, fd[0]);
    CGI *obs = makeCGI(fd[0], pid);
    em_->add(std::pair<t_id, t_type>(fd[0], FD), obs);
    em_->disableReadEvent(id_);
    em_->disableTimerEvent(id_);
    em_->registerWriteEvent(fd[0]);
  }
}

void ConnectionSocket::processDELETE() {
  if (!isAcceptableMethod(loc_conf_, HttpRequest::DELETE)) {
    throw MethodNotAllowedException("DELETE is not allowed in this Location scope");
  }
  std::string path = loc_conf_->getTargetPath(request_.request_target_->getPath());

  // if CGI extension exist, try exec CGI
  const bool hasCGI = CGIextension_ != "";
  if (hasCGI && contain(loc_conf_->cgi_exts_, CGIextension_)) {
    execCGI(path);
    return;
  }
  throw ResourceForbiddenException("DELETE must be processed by cgi only");
}

void ConnectionSocket::processPOST() {
  if (!isAcceptableMethod(loc_conf_, HttpRequest::POST)) {
    throw MethodNotAllowedException("POST is not allowed in this Location scope");
  }
  std::string path = loc_conf_->getTargetPath(request_.request_target_->getPath());
  // if CGI extension exist, try exec CGI
  const bool hasCGI = CGIextension_ != "";
  if (hasCGI && contain(loc_conf_->cgi_exts_, CGIextension_)) {
    execCGI(path);
    return;
  }
  throw ResourceForbiddenException("POST must be processed by cgi only");
}

void ConnectionSocket::processGET() {
  if (!isAcceptableMethod(loc_conf_, HttpRequest::GET)) {
    throw MethodNotAllowedException("No Suitable Location");
  }
  std::string path = loc_conf_->getTargetPath(request_.request_target_->getPath());

  // if CGI extension exist, try exec CGI
  const bool hasCGI = CGIextension_ != "";
  if (hasCGI && contain(loc_conf_->cgi_exts_, CGIextension_)) {
    execCGI(path);
    return;
  }
  struct stat st;
  if (stat(path.c_str(), &st) == -1) {
    throw ResourceNotFoundException("stat error: file doesn't exist");  // 404 Not Found
  }
  bool is_directory = (st.st_mode & S_IFMT) == S_IFDIR;
  // check directory or file is readable
  if (!isReadable(path.c_str())) {
    throw ResourceForbiddenException("access Read error");  // 403 Forbiden
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
      response_.appendBody(GET::listFilesAndDirectories(path, request_));
      response_.setStatusAndReason(200);
      response_.setContentType("autoindex.html", false);
      em_->disableReadEvent(id_);
      em_->registerWriteEvent(id_);
      return;  // 200 OK
    }
    if (idx_path == "" && !loc_conf_->common_.autoindex_) {
      throw ResourceNotFoundException("no index and autoindex");  // 404 Not Found
    }
  }
  // URI file or index file
  response_.setContentType(path, false);
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) throw InternalServerErrorException("open error");
  GET *obs = makeGET(fd);
  em_->add(std::pair<t_id, t_type>(fd, FD), obs);
  em_->disableReadEvent(id_);
  em_->disableTimerEvent(id_);
  em_->registerReadEvent(fd);
}

void ConnectionSocket::processRedirect() {
  if (!isAcceptableMethod(loc_conf_, request_.method_)) {
    throw MethodNotAllowedException("No Suitable Location");
  }
  response_.setStatusAndReason(std::atoi(loc_conf_->getRedirectStatus().c_str()));
  response_.appendHeader("location", loc_conf_->getRedirectURI());
  em_->disableReadEvent(id_);
  em_->registerWriteEvent(id_);
}

void ConnectionSocket::processErrorPage(CommonConf *common_conf) {
  DEBUG_PUTS("process ErrorPage");
  std::stringstream ss;
  ss << response_.getStatus();
  if (!common_conf) {
    // Bad Request error page
    response_.appendBody(HTML::getDefaultErrorPage(ss.str(), conf_.cache_.statusMsg_[response_.getStatus()]));
    response_.appendHeader("content-type", "text/html");
  } else {
    // Other error page
    std::map<std::string, std::string>::iterator itr = common_conf->error_pages_.find(ss.str());
    if (itr != common_conf->error_pages_.end()) {
      std::string filename = itr->second;
      if (filename[0] != '/') filename = common_conf->root_ + "/" + filename;
      if (conf_.cache_.error_page_paths_.find(filename) != conf_.cache_.error_page_paths_.end()) {
        response_.appendBody(conf_.cache_.error_page_paths_[filename]);
        response_.setContentType(conf_.cache_.error_page_paths_[filename], true);
        return;
      }
    }
    response_.appendBody(HTML::getDefaultErrorPage(ss.str(), conf_.cache_.statusMsg_[response_.getStatus()]));
    response_.appendHeader("content-type", "text/html");
  }
}

void ConnectionSocket::process() {
  ServerConf *serv_conf = conf_.getServerConf(port_, request_.headers_.host.uri_host);
  loc_conf_ = serv_conf->getLocationConf(&request_);
  // loc_conf is redirection block
  if (loc_conf_->hasRedirectDirective()) {
    processRedirect();
    return;
  }
  CGIextension_ = getCGIExtension(request_.request_target_->getPath());
  if (request_.methodIs(HttpRequest::GET)) {
    processGET();
  } else if (request_.methodIs(HttpRequest::POST)) {
    processPOST();
  } else if (request_.methodIs(HttpRequest::DELETE)) {
    processDELETE();
  }
  DEBUG_PUTS("PROCESSING FINISHED");
}

void ConnectionSocket::notify(struct kevent ev) {
  DEBUG_PUTS("ConnectionSocket notify");
  em_->updateTimer(id_);
  if (ev.filter == EVFILT_READ) {
    DEBUG_PUTS("handle_request() called");
    try {
      HttpRequestReader::State state = rreader_.read();
      if (state == HttpRequestReader::FinishedReading) {
        DEBUG_PRINTF("FINISHED READING: %s \n",
                     escape(std::string(request_.body_.begin(), request_.body_.end())).c_str());
        this->process();
      }
    } catch (HttpException &e) {
      // all 4xx 5xx exception(readRequest and process) is caught here
      DEBUG_PUTS(e.what());
      response_ = HttpResponse(id_, port_, &conf_);
      response_.setStatusAndReason(e.statusCode());
      // todo: when exception is raised in request reading, loc_conf_ is not sent
      ServerConf *serv_conf = conf_.getServerConf(port_, request_.headers_.host.uri_host);
      CommonConf *common_conf = (loc_conf_ ? &loc_conf_->common_ : &serv_conf->common_);
      processErrorPage(common_conf);
      em_->disableReadEvent(id_);
      em_->registerWriteEvent(id_);
    } catch (std::runtime_error &e) {
      DEBUG_PUTS("client close socket");
      shutdown();
    }
  }
  if (ev.filter == EVFILT_WRITE) {
    DEBUG_PUTS("handle_response() called");
    response_.createResponse();
    try {
      response_.sendResponse();
    } catch (std::runtime_error &e) {
      DEBUG_PUTS("client close socket");
      shutdown();
      return;
    }
    if (response_.getState() == HttpResponse::End) {
      loc_conf_ = NULL;
      CGIextension_ = "";
      request_ = HttpRequest();
      rreader_ = HttpRequestReader(rreader_, request_);
      response_ = HttpResponse(id_, port_, &conf_);
      em_->disableWriteEvent(id_);
      em_->registerReadEvent(id_);
    }
  }
}
