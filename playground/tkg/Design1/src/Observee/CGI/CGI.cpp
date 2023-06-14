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

#include "../../Config/validation.h"
#include "../../HttpException.hpp"
#include "../../helper.hpp"
#include "../HttpRequest/HttpRequestReader.hpp"
#include "helper.hpp"

void CGI::shutdown() {
  DEBUG_PUTS("CGI shutdown");
  close(id_);
  em_->deleteTimerEvent(id_);
  kill(pid_, SIGINT);
  waitpid(pid_, NULL, 0);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

std::pair<std::string, std::string> getHeaderField(std::string &field) {
  std::stringstream ss(field);
  std::string name, value;
  if (field.find(':') == std::string::npos) throw std::runtime_error("invalid header field");
  std::getline(ss, name, ':');
  std::getline(ss, value);
  value = trimOws(value);
  return std::pair<std::string, std::string>(name, value);
}

void CGI::parseDocRes(std::vector<std::string> &lines) {
  std::size_t i = 0;
  while (i < lines.size() && lines[i] != "") {
    t_field field = getHeaderField(lines[i]);
    if (i == 0) {
      response_->appendHeader("Content-Type", field.second);
    } else if (i == 1 && field.first == "Status") {
      response_->setStatus(std::atoi(field.second.c_str()));
    } else {
      response_->appendHeader(field.first, field.second);
    }
    i++;
  }
  // parse body
  i++;
  for (; i < lines.size(); i++) {
    response_->appendBody(lines[i] + "\n");
  }
  return;
}

bool CGI::isDocRes(std::vector<std::string> &lines) {
  try {
    std::size_t i = 0;
    while (i < lines.size() && lines[i] != "") {
      t_field field = getHeaderField(lines[i]);
      if (i == 0 && field.first != "Content-Type") return false;
      if (i == 1 && field.first == "Status" && !isStatusCode(field.second)) return false;
      i++;
    }
  } catch (std::runtime_error &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool CGI::isLocalRedirectRes(std::vector<std::string> &lines) {
  if (lines.size() != 1) return false;
  t_field field = getHeaderField(lines[0]);
  if (field.first != "Location") return false;
  if (!CGIValidation::isAbsPath(field.second.c_str())) return false;
  return true;
}

bool CGI::isClientRedirectRes(std::vector<std::string> &lines) {
  // todo:
  if (lines.size() != 1) return false;
  t_field filed = getHeaderField(lines[0]);
  if (filed.first != "Location") return false;
  if (!CGIValidation::isAbsURI(filed.second)) return false;
  return true;
}

bool CGI::isClientRedirectWithDocRes(std::vector<std::string> &lines) {
  // todo:
  (void)lines;
  return false;
}

CGI::Type CGI::getResponseType(std::vector<std::string> &lines) {
  if (isDocRes(lines)) {
    return CGI::Doc;
  } else if (isLocalRedirectRes(lines)) {
    return CGI::LocalRedir;
  } else if (isClientRedirectRes(lines)) {
    return CGI::ClientRedir;
  } else if (isClientRedirectWithDocRes(lines)) {
    return CGI::ClientRedirWithDoc;
  }
  return CGI::Error;
}

int CGI::parseCGIResponse() {
  // todo:
  std::vector<std::string> lines = CGIValidation::ExtractLines(recieve_data_);
  CGI::Type type = getResponseType(lines);
  if (type == CGI::Doc) {
    parseDocRes(lines);
  } else if (type == CGI::LocalRedir) {
    // todo: parse and localredirect
  } else if (type == CGI::ClientRedir) {
    // todo: parse and clientRedir
  } else if (type == CGI::ClientRedirWithDoc) {
    // todo: parse and ClientRdir with Doc
  }
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
      close(id_);
      waitpid(pid_, &status, 0);
      if (status == 0) {
        response_->setStatus(200);
        // todo: analyse CGI response and then
        // switch document-response | local-redir-response | client-redir-response | client-redirdoc-response process
        parseCGIResponse();
      } else
        response_->setStatus(500);
      parent_->obliviateChild(this);
      em_->deleteTimerEvent(id_);
      em_->registerWriteEvent(parent_->id_);
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
      perror("sendto");
      response_->setStatus(501);
      em_->registerWriteEvent(parent_->id_);
      shutdown();
      return;
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
