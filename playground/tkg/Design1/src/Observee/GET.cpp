#include "GET.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
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

#include "../Config/Config.hpp"
#include "../HTML.hpp"
#include "../HttpException.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse.hpp"

void GET::timeout() {
  response_ = dynamic_cast<ConnectionSocket *>(parent_)->initResponse();
  response_->setStatusAndReason(500);
  em_->enableTimerEvent(parent_->id_);
  em_->registerWriteEvent(parent_->id_);
  parent_->stopMonitorChild(this);
  parent_ = NULL;
  for (std::vector<Observee *>::iterator itr = children_.begin(); itr != children_.end(); itr++) {
    (*itr)->parent_ = NULL;
    (*itr)->shutdown();
  }
  children_.clear();
  close(id_);
  em_->deleteTimerEvent(id_);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void GET::shutdown() {
  DEBUG_PUTS("GET shutdown");
  em_->enableTimerEvent(parent_->id_);
  parent_->stopMonitorChild(this);
  parent_ = NULL;
  for (std::vector<Observee *>::iterator itr = children_.begin(); itr != children_.end(); itr++) {
    (*itr)->parent_ = NULL;
    (*itr)->shutdown();
  }
  children_.clear();
  close(id_);
  em_->deleteTimerEvent(id_);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void GET::terminate() { close(id_); }

std::string GET::listFilesAndDirectories(std::string &directory_path, const HttpRequest &req) {
  DIR *dir;
  struct dirent *entry;
  struct stat file_stat;
  std::string ret = "";

  if ((dir = opendir(directory_path.c_str())) == NULL) {
    throw InternalServerErrorException("listFilesAndDirectories: opendir error");
  }
  ret += HTML::header();
  ret += "<h1>Index of</h1>";
  while ((entry = readdir(dir)) != NULL) {
    std::string file_path;
    std::string target_path;
    if (directory_path.back() != '/') {
      file_path = directory_path + "/" + entry->d_name;
      target_path = req.request_target_->getPath() + "/" + entry->d_name;
    } else {
      file_path = directory_path + entry->d_name;
      target_path = req.request_target_->getPath() + entry->d_name;
    }
    if (stat(file_path.c_str(), &file_stat) == -1) {
      perror("listFilesAndDirectories: stat error");
      continue;
    }
    std::stringstream ss;
    ss << "http://" << req.headers_.host.uri_host << ":" << req.headers_.host.port << target_path;
    std::string name = entry->d_name;
    if (S_ISREG(file_stat.st_mode)) {
      ret += HTML::aTag(HTML::sanitize(ss.str()), HTML::sanitize(name));
    } else if (S_ISDIR(file_stat.st_mode)) {
      if (name != "." && name != "..") {
        name += "/";
        ret += HTML::aTag(HTML::sanitize(ss.str()), HTML::sanitize(name));
      }
    }
  }
  ret += HTML::footer();
  closedir(dir);
  return ret;
}

void GET::notify(struct kevent ev) {
  DEBUG_PUTS("handle GET");
  (void)ev;
  char buff[FILE_READ_SIZE + 1];
  int res = read(id_, buff, FILE_READ_SIZE);
  DEBUG_PRINTF("res: %d\n", res);
  if (res == -1) {
    response_->setStatusAndReason(500, "");
    em_->registerWriteEvent(parent_->id_);
    shutdown();
    return;
  } else {
    response_->appendBody(buff, res);
    if (res == 0 || res == ev.data) {
      DEBUG_PRINTF("GET LAST RESULT: '%s'\n",
                   std::string(&(response_->getBody()[0]), response_->getBody().size()).c_str());
      response_->setStatusAndReason(200);
      em_->registerWriteEvent(parent_->id_);
      shutdown();
      return;
    }
    DEBUG_PRINTF("GET WIP RESULT: '%s'\n",
                 std::string(&(response_->getBody()[0]), response_->getBody().size()).c_str());
  }
}
