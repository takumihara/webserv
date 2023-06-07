#include "GET.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
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

#include "../Config/Config.hpp"
#include "../HttpException.hpp"
#include "../HttpResponse.hpp"

void GET::shutdown() {
  DEBUG_PUTS("GET shutdown");
  close(id_);
  em_->deleteTimerEvent(id_);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

std::string GET::listFilesAndDirectories(const std::string &directory_path) {
  DIR *dir;
  struct dirent *entry;
  struct stat file_stat;
  std::string ret = "";

  if ((dir = opendir(directory_path.c_str())) == NULL) {
    throw InternalServerErrorException("listFilesAndDirectories: opendir error");
  }
  while ((entry = readdir(dir)) != NULL) {
    std::string file_path;
    file_path = directory_path + "/" + entry->d_name;

    if (stat(file_path.c_str(), &file_stat) == -1) {
      perror("listFilesAndDirectories: stat error");
      continue;
    }
    std::string name = entry->d_name;
    if (S_ISREG(file_stat.st_mode)) {
      ret += entry->d_name;
      ret += "\n";
    } else if (S_ISDIR(file_stat.st_mode)) {
      if (name != "." && name != "..") {
        ret += entry->d_name;
        ret += "/\n";
      }
    }
  }
  closedir(dir);
  return ret;
}

void GET::notify(struct kevent ev) {
  std::cout << "handle GET" << std::endl;
  std::stringstream ss;
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
      ss << response_->getBody().size();
      response_->appendHeader("Content-Length", ss.str());
      response_->setStatus(200);
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
