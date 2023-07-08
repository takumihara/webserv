#include "POST.hpp"

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

void POST::timeout() {
  DEBUG_PUTS("POST shutdown");
  close(id_);
  parent_->stopMonitorChild(this);
  em_->deleteTimerEvent(id_);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void POST::shutdown() {
  DEBUG_PUTS("POST shutdown");
  close(id_);
  parent_->stopMonitorChild(this);
  em_->deleteTimerEvent(id_);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void POST::terminate() { close(id_); }

void POST::notify(struct kevent ev) {
  DEBUG_PUTS("handle POST");
  (void)ev;
  std::size_t size = body_size_ - write_size_;
  if (size > FILE_WRITE_SIZE) size = FILE_WRITE_SIZE;
  int res = write(id_, &(request_->body_[write_size_]), size);
  if (res == -1) {
    return;
  } else {
    write_size_ += res;
    if (write_size_ >= request_->body_.size()) {
      close(id_);
      parent_->stopMonitorChild(this);
      em_->deleteTimerEvent(id_);
      em_->registerWriteEvent(parent_->id_);
      em_->remove(std::pair<t_id, t_type>(id_, FD));
      return;
    }
  }
}
