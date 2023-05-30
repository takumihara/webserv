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

std::string ConnectionSocket::getTargetPath(const LocationConf &loc) {
  std::string target = loc.common_.root_;
  if (target[target.size() - 1] == '/' && request_.getRequestTarget().absolute_path[0] == '/')
    target.erase(target.size() - 1);
  target += request_.getRequestTarget().absolute_path;
  return target;
}

std::string ConnectionSocket::listFilesAndDirectories(const std::string &directory_path) {
  DIR *dir;
  struct dirent *entry;
  struct stat fileStat;
  std::string ret = "";

  if ((dir = opendir(directory_path.c_str())) == NULL) {
    perror("Error opening directory");
    return ret;
  }
  while ((entry = readdir(dir)) != NULL) {
    std::string file_path;
    file_path = directory_path + "/" + entry->d_name;

    if (stat(file_path.c_str(), &fileStat) == -1) {
      perror("Error getting file/directory information");
      continue;
    }
    std::string name = entry->d_name;
    if (S_ISREG(fileStat.st_mode)) {
      ret += entry->d_name;
      ret += "\n";
    } else if (S_ISDIR(fileStat.st_mode)) {
      if (name != "." && name != "..") {
        ret += entry->d_name;
        ret += "/\n";
      }
    }
  }
  std::cout << "directory list: " << ret;
  closedir(dir);
  return ret;
}

std::string ConnectionSocket::getIndexFile(const LocationConf &conf, std::string path) {
  std::vector<std::string> index = conf.common_.index_;

  if (path[path.size() - 1] != '/') path += "/";
  for (std::vector<std::string>::const_iterator itr = index.cbegin(); itr != index.cend(); itr++) {
    if (access((path + (*itr)).c_str(), R_OK) == 0) {
      return path + *itr;
    }
  }
  return "";
}

void ConnectionSocket::processGET(EventManager &event_manager, std::string path) {
  const ServerConf *serv_conf = conf_.getServerConf(port_, request_.getHost().uri_host);
  const LocationConf &loc_conf = conf_.getLocationConf(serv_conf, request_.getRequestTarget().absolute_path);

  (void)event_manager;
  // check directory or file exists
  struct stat st;
  if (stat(path.c_str(), &st) == -1) {
    response_.setStatus(404);
    DEBUG_PUTS("stat error");
    return;  // 404 error
  }
  // check directory or file is readable
  if (access(path.c_str(), R_OK) != 0) {
    response_.setStatus(403);
    DEBUG_PUTS("access error");
    return;  // return 403 Forbiden
  }
  bool is_directory = (st.st_mode & S_IFMT) == S_IFDIR;
  if (is_directory) {
    // see through index files (if no index files and autoindex is on, you should create directory list)
    std::string idx_path;
    if (loc_conf.common_.index_.size() != 0) {
      idx_path = getIndexFile(loc_conf, path);
      if (idx_path != "") path = idx_path;
    }
    if (idx_path == "" && loc_conf.common_.autoindex_) {
      result_ = listFilesAndDirectories(path);
      response_.setStatus(200);
      DEBUG_PUTS("autoindex");
      return;  // 200 OK
    }
    if (idx_path == "" && !loc_conf.common_.autoindex_) {
      response_.setStatus(404);
      DEBUG_PUTS("no index and autoindex");
      return;  // 404 error
    }
  }
  result_ = readFile(path.c_str());
  DEBUG_PUTS("index or autoindex");
  response_.setStatus(200);
  return;  // 200 OK
}

void ConnectionSocket::process(EventManager &event_manager) {
  const ServerConf *serv_conf = conf_.getServerConf(port_, request_.getHost().uri_host);
  const LocationConf &loc_conf = conf_.getLocationConf(serv_conf, request_.getRequestTarget().absolute_path);

  // todo: check if file exists
  std::string path = "." + getTargetPath(loc_conf);
  if (path.find(".cgi") != std::string::npos) {
    execCGI(path, event_manager);
  } else if (request_.request_line_.method == HttpRequest::GET) {
    // handle GET
    processGET(event_manager, path);
    event_manager.addChangedEvents((struct kevent){id_, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0});
    event_manager.addChangedEvents(
        (struct kevent){id_, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
    return;
  } else if (request_.request_line_.method == HttpRequest::POST) {
    // handle POST
  } else if (request_.request_line_.method == HttpRequest::DELETE) {
    // handle DELETE
  }
  DEBUG_PUTS("PROCESSING FINISHED");
}

void ConnectionSocket::notify(EventManager &event_manager, struct kevent ev) {
  DEBUG_PUTS("ConnectionSocket notify");
  if (ev.filter == EVFILT_READ) {
    DEBUG_PUTS("handle_request() called");
    try {
      bool finished_reading = request_.readRequest(event_manager);
      if (finished_reading) {
        this->process(event_manager);
      }
      // todo(thara): handle exceptions
      // } catch (const HttpRequest::BadRequestException &e) {
      // } catch (const HttpRequest::NotImplementedException &e) {
      // } catch (const HttpRequest::NotAllowedException &e) {
      // } catch (const HttpRequest::VersionNotSupportedException &e) {
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      close(ev.ident);
      event_manager.remove(std::pair<t_id, t_type>(ev.ident, FD));
      event_manager.addChangedEvents((struct kevent){ev.ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
    } catch (std::runtime_error &e) {
      std::cout << e.what() << std::endl;
      close(ev.ident);
      event_manager.remove(std::pair<t_id, t_type>(ev.ident, FD));
      event_manager.addChangedEvents((struct kevent){ev.ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
    }
  }
  if (ev.filter == EVFILT_WRITE) {
    DEBUG_PUTS("handle_response() called");
    request_.refresh();
    response_.createResponse(result_);
    response_.sendResponse(event_manager);
    // request_ = HttpRequest(id_, port_, conf_);
    // request_.headers_ .clear();
  }
}
