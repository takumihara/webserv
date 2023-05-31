#include "CGI.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
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

void CGI::shutdown() {
  DEBUG_PUTS("CGI shutdown");
  close(id_);
  em_->deleteTimerEvent(id_);
  kill(pid_, SIGINT);
  waitpid(pid_, NULL, 0);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void CGI::notify(struct kevent ev) {
  std::cout << "handle CGI" << std::endl;
  (void)ev;
  char buff[FILE_READ_SIZE + 1];
  int res = read(id_, &buff[0], FILE_READ_SIZE);
  if (ev.flags & EV_EOF) std::cout << "CGI EOF" << std::endl;
  if (res == -1)
    return;
  else if (res == 0) {
    close(id_);
    response_->setStatus(200);
    parent_->obliviateChild(this);
    em_->deleteTimerEvent(id_);
    em_->registerWriteEvent(parent_->id_);
    std::cout << "CGI LAST RESULT: '" << response_->getBody() << "'" << std::endl;
    em_->remove(std::pair<t_id, t_type>(id_, FD));
  } else {
    std::cout << "res: " << res << std::endl;
    buff[res] = '\0';
    response_->appendBody(std::string(buff));
    em_->updateTimer(this);
    std::cout << "cgi wip result: '" << response_->getBody() << "'" << std::endl;
  }
}
