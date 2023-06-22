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

void GET::shutdown() {
  DEBUG_PUTS("GET shutdown");
  close(id_);
  em_->deleteTimerEvent(id_);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

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
    if (directory_path.back() != '/')
      file_path = directory_path + "/" + entry->d_name;
    else
      file_path = directory_path + entry->d_name;
    if (stat(file_path.c_str(), &file_stat) == -1) {
      perror("listFilesAndDirectories: stat error");
      continue;
    }
    std::string name = entry->d_name;
    std::stringstream ss;
    // todo: this if statment and rmed_dot are temporary
    std::string rmed_dot;
    if (file_path[0] == '.') rmed_dot = file_path.substr(1);
    ss << "http://" << req.getHost().uri_host << ":" << req.getHost().port << rmed_dot;
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
  std::cout << "handle GET" << std::endl;
  (void)ev;
  char buff[FILE_READ_SIZE + 1];
  int res = read(id_, &buff[0], FILE_READ_SIZE);
  std::cout << "res: " << res << std::endl;
  if (res == -1) {
    return;
  } else {
    buff[res] = '\0';
    response_->appendBody(std::string(buff));
    if (res == 0 || res == ev.data) {
      close(id_);
      response_->setStatusAndReason(200, "");
      parent_->obliviateChild(this);
      em_->deleteTimerEvent(id_);
      em_->registerWriteEvent(parent_->id_);
      std::cout << "GET LAST RESULT: '" << response_->getBody() << "'" << std::endl;
      em_->remove(std::pair<t_id, t_type>(id_, FD));
      return;
    }
    em_->updateTimer(this);
    std::cout << "GET wip result: '" << response_->getBody() << "'" << std::endl;
  }
}
