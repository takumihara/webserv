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

void CGI::shutdown(EventManager &em) {
  DEBUG_PUTS("CGI shutdown");
  close(id_);
  em.addChangedEvents((struct kevent){static_cast<uintptr_t>(id_), EVFILT_TIMER, EV_DELETE, 0, 0, NULL});
  kill(pid_, SIGINT);
  waitpid(pid_, NULL, 0);
  em.remove(std::pair<t_id, t_type>(id_, FD));
}

void CGI::notify(EventManager &event_manager, struct kevent ev) {
  std::cout << "handle CGI" << std::endl;
  (void)ev;
  char buff[FILE_READ_SIZE + 1];
  int res = read(id_, &buff[0], FILE_READ_SIZE);
  if (res == -1)
    return;
  else if (res == 0) {
    close(id_);
    parent_->obliviateChild(this);
    (void)pid_;
    event_manager.addChangedEvents(

        (struct kevent){static_cast<uintptr_t>(id_), EVFILT_TIMER, EV_DELETE, NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
    event_manager.addChangedEvents(
        (struct kevent){static_cast<uintptr_t>(parent_->id_), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, 0});
    event_manager.addChangedEvents((struct kevent){static_cast<uintptr_t>(parent_->id_), EVFILT_TIMER,
                                                   EV_ADD | EV_ENABLE, NOTE_SECONDS, EventManager::kTimeoutDuration,
                                                   0});
    std::cout << "CGI LAST RESULT: '" << *result_ << "'" << std::endl;
    event_manager.remove(std::pair<t_id, t_type>(id_, FD));
  } else {
    std::cout << "res: " << res << std::endl;
    buff[res] = '\0';
    response_->appendBody(std::string(buff));
    std::cout << "cgi wip result: '" << response_->getBody() << "'" << std::endl;
  }
}
