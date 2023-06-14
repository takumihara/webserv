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
#include <sstream>
#include <stdexcept>

#include "../HttpRequest/HttpRequestReader.hpp"
#include "../helper.hpp"

void CGI::shutdown() {
  DEBUG_PUTS("CGI shutdown");
  close(id_);
  em_->deleteTimerEvent(id_);
  kill(pid_, SIGINT);
  waitpid(pid_, NULL, 0);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

std::vector<std::string> ExtractLines(const std::string &data) {
  std::vector<std::string> lines;
  std::string str;
  std::stringstream ss(data);
  while (getline(ss, str)) {
    lines.push_back(str);
  }
  return lines;
}

bool CGI::isDocRes(std::vector<std::string> &lines) {
  // todo:
  (void)lines;
  return false;
}

bool CGI::isLocalRedirectRes(std::vector<std::string> &lines) {
  // todo:
  if (lines.size() != 1) return false;
  std::stringstream ss(lines[0]);
  std::string name, value;
  std::getline(ss, name, ':');
  std::getline(ss, value);
  value = trimOws(value);
  if (name != "Location") return false;
  // if (isValid)
  return false;
}

bool CGI::isClientRedirectRes(std::vector<std::string> &lines) {
  // todo:
  (void)lines;
  return false;
}

bool CGI::isClientRedirectWithDocRes(std::vector<std::string> &lines) {
  // todo:
  (void)lines;
  return false;
}

CGI::Type CGI::getResponseType(std::vector<std::string> &lines) {
  if (isDocRes(lines)) {
    // todo: document response type
  } else if (isLocalRedirectRes(lines)) {
  }
  return Doc;
}

int CGI::parseCGIResponse() {
  // todo:
  std::vector<std::string> lines = ExtractLines(recieve_data_);
  getResponseType(lines);
  return 1;
}

void CGI::notify(struct kevent ev) {
  std::cout << "handle CGI" << std::endl;
  (void)recieved_size_;

  if (ev.filter == EVFILT_READ) {
    char buff[FILE_READ_SIZE + 1];
    int status;

    int res = read(id_, &buff[0], FILE_READ_SIZE);
    if (res == -1) {
      // todo: set response staus 5xx and delete this CGI observee
      std::cout << "res = -1" << std::endl;
      return;
    } else if (res == 0) {
      // todo: analyse CGI response and then
      // switch document-response | local-redir-response | client-redir-response | client-redirdoc-response process
      parseCGIResponse();
      close(id_);
      waitpid(pid_, &status, 0);
      if (status == 0) {
        response_->setStatus(200);
      } else
        response_->setStatus(500);
      parent_->obliviateChild(this);
      em_->deleteTimerEvent(id_);
      em_->registerWriteEvent(parent_->id_);
      response_->appendBody(recieve_data_);
      std::cout << "CGI LAST RESULT: '" << recieve_data_ << "'" << std::endl;
      em_->remove(std::pair<t_id, t_type>(id_, FD));
    } else {
      std::cout << "res: " << res << std::endl;
      buff[res] = '\0';
      recieve_data_ += buff;
      em_->updateTimer(this);
      std::cout << "cgi wip result: '" << recieve_data_ << "'" << std::endl;
    }
  } else if (ev.filter == EVFILT_WRITE) {
    const char *response = request_->getBody().c_str();
    std::cout << "response: " << response << std::endl;
    std::size_t size = request_->getBody().size() - sending_size_;
    if (size > SOCKET_WRITE_SIZE) {
      size = SOCKET_WRITE_SIZE;
    }
    int res = write(id_, &response[sending_size_], size);
    if (res == -1) {
      // todo: set response staus 5xx and delete this CGI observee
      perror("sendto");
      throw std::runtime_error("send error");
    }
    sending_size_ += size;
    if (sending_size_ == request_->getBody().size()) {
      // todo: temporary suhtdown()
      ::shutdown(id_, SHUT_WR);
      em_->disableWriteEvent(id_);
      em_->registerReadEvent(id_);
    }
  }
}
