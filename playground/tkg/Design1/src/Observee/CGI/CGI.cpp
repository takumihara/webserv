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
  int status = 0;
  close(id_);
  em_->deleteTimerEvent(id_);
  kill(pid_, SIGTERM);
  waitpid(pid_, &status, 0);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
  if (status != 0) response_->setStatusAndReason(500, "");
}

void CGI::terminate() {
  close(id_);
  kill(pid_, SIGTERM);
  waitpid(pid_, NULL, 0);
}

std::pair<std::string, std::string> getHeaderField(std::string &field) {
  std::stringstream ss(field);
  std::string name, value;
  if (field.find(':') == std::string::npos) throw std::runtime_error("invalid header field");
  std::getline(ss, name, ':');
  std::getline(ss, value);
  return std::pair<std::string, std::string>(name, value);
}

void CGI::parseDocRes(std::vector<std::string> &lines) {
  DEBUG_PUTS("parseDocRes");
  std::size_t i = 0;
  while (i < lines.size() && lines[i] != "") {
    t_field field = getHeaderField(lines[i]);
    if (i == 0) {
      response_->appendHeader("content-type", field.second);
    } else if (i == 1 && field.first == "Status") {
      std::istringstream iss(field.second);
      std::string status;
      std::string reason;
      iss >> status;
      iss.ignore();
      getline(iss, reason, '\n');
      response_->setStatusAndReason(std::atoi(status.c_str()), reason);
    } else {
      response_->appendHeader(toLower(field.first), field.second);
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

void CGI::parseClientRedirect(std::vector<std::string> &lines) {
  DEBUG_PUTS("parseClientRedirect");
  t_field field = getHeaderField(lines[0]);
  response_->appendHeader("location", field.second);
  response_->setStatusAndReason(302, "");
}

void CGI::parseClientRedirectWithDoc(std::vector<std::string> &lines) {
  DEBUG_PUTS("parseClientRedirectWithDoc");
  t_field field = getHeaderField(lines[0]);
  response_->appendHeader("location", field.second);
  field = getHeaderField(lines[1]);
  response_->setStatusAndReason(std::atoi(field.second.c_str()), field.second.substr(4));
  field = getHeaderField(lines[2]);
  response_->appendHeader("content-type", field.second);
  std::size_t i = 3;
  while (i < lines.size() && lines[i] != "") {
    field = getHeaderField(lines[i]);
    response_->appendHeader(toLower(field.first), field.second);
    i++;
  }
  if (lines[i] == "") i++;
  while (i < lines.size() && lines[i] != "") {
    response_->appendBody(lines[i] + "\n");
    i++;
  }
}

void CGI::parseLocalRedirect(std::vector<std::string> &lines) {
  DEBUG_PUTS("parseLocalRedirect");
  t_field field = getHeaderField(lines[0]);
  try {
    const size_t fragment_pos = field.second.find("#");
    std::string fragment;
    if (fragment_pos != std::string::npos) {
      fragment = field.second.substr(fragment_pos + 1);
    }
    URI *uri = URI::parseRequestURI(field.second.substr(0, fragment_pos));
    delete request_->request_target_;
    request_->request_target_ = uri;
    request_->body_.clear();
    dynamic_cast<ConnectionSocket *>(parent_)->initExtension();
  } catch (std::runtime_error &e) {
    throw(InternalServerErrorException("CGI response URI is invalid"));
  }
}

bool CGI::isDocRes(std::vector<std::string> &lines) {
  try {
    std::size_t i = 0;
    while (i < lines.size() && lines[i] != "") {
      t_field field = getHeaderField(lines[i]);
      if (i == 0 && field.first != "Content-Type") return false;
      if (i == 1 && field.first == "Status" && !CGIValidation::isStatusHeaderValue(field.second)) return false;
      i++;
    }
  } catch (std::runtime_error &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool CGI::isLocalRedirectRes(std::vector<std::string> &lines) {
  if (lines.size() != 2) return false;
  try {
    t_field field = getHeaderField(lines[0]);
    if (field.first != "Location") return false;
    if (!CGIValidation::isAbsPath(field.second.c_str())) return false;
    if (lines[1] != "") return false;
  } catch (std::runtime_error &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool CGI::isClientRedirectRes(std::vector<std::string> &lines) {
  if (lines.size() != 2) return false;
  try {
    t_field filed = getHeaderField(lines[0]);
    if (filed.first != "Location") return false;
    if (!CGIValidation::isAbsURI(filed.second)) return false;
    if (lines[1] != "") return false;
  } catch (std::runtime_error &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
}

bool CGI::isClientRedirectWithDocRes(std::vector<std::string> &lines) {
  if (lines.size() < 3) return false;
  try {
    t_field field = getHeaderField(lines[0]);
    if (field.first != "Location" || !CGIValidation::isAbsURI(field.second)) return false;
    field = getHeaderField(lines[1]);
    if (field.first != "Status" || !CGIValidation::isStatusHeaderValue(field.second)) return false;
    field = getHeaderField(lines[2]);
    if (field.first != "Content-Type" || !CGIValidation::isMediaType(field.second)) return false;
    std::size_t i = 3;
    while (i < lines.size() && lines[i] != "") {
      field = getHeaderField(lines[i]);
      i++;
    }
    if (i == lines.size() && lines[i - 1] != "") return false;
  } catch (std::runtime_error &e) {
    std::cout << e.what() << std::endl;
    return false;
  }
  return true;
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

void CGI::parseCGIResponse() {
  std::vector<std::string> lines = CGIValidation::extractLines(recieved_data_);
  CGI::Type type = getResponseType(lines);
  if (type == CGI::Error) {
    response_->setStatusAndReason(500, "");
  } else if (type == CGI::Doc) {
    parseDocRes(lines);
  } else if (type == CGI::LocalRedir) {
    DEBUG_PRINTF("CGI LAST RESULT: '%s'", recieved_data_.c_str());
    try {
      parseLocalRedirect(lines);
      ConnectionSocket *parent = dynamic_cast<ConnectionSocket *>(parent_);
      shutdown();
      parent->process();
    } catch (HttpException &e) {
      response_->setStatusAndReason(e.statusCode(), "");
      type = CGI::Error;
    }
  } else if (type == CGI::ClientRedir) {
    parseClientRedirect(lines);
  } else if (type == CGI::ClientRedirWithDoc) {
    parseClientRedirectWithDoc(lines);
  }
  if (type != CGI::LocalRedir) {
    DEBUG_PRINTF("CGI LAST RESULT: '%s'", recieved_data_.c_str());
    em_->registerWriteEvent(parent_->id_);
    shutdown();
  }
}

void CGI::notify(struct kevent ev) {
  DEBUG_PUTS("handle CGI");
  (void)recieved_size_;

  if (ev.filter == EVFILT_READ) {
    char buff[FILE_READ_SIZE + 1];

    int res = read(id_, &buff[0], FILE_READ_SIZE);
    if (res == -1) {
      response_->setStatusAndReason(501, "");
      em_->registerWriteEvent(parent_->id_);
      shutdown();
      return;
    } else if (res == 0) {
      response_->setStatusAndReason(200, "");
      parseCGIResponse();
    } else {
      std::cout << "res: " << res << std::endl;
      buff[res] = '\0';
      recieved_data_ += buff;
      em_->updateTimer(this);
      DEBUG_PRINTF("CGI WIP RESULT: '%s'", recieved_data_.c_str());
    }
  } else if (ev.filter == EVFILT_WRITE) {
    const char *response = request_->body_.c_str();
    std::cout << "response: " << response << std::endl;
    std::size_t size = request_->body_.size() - sending_size_;
    if (size > SOCKET_WRITE_SIZE) {
      size = SOCKET_WRITE_SIZE;
    }
    int res = write(id_, &response[sending_size_], size);
    if (res == -1) {
      perror("sendto");
      response_->setStatusAndReason(501, "");
      em_->registerWriteEvent(parent_->id_);
      shutdown();
      return;
    }
    sending_size_ += size;
    if (sending_size_ == request_->body_.size()) {
      ::shutdown(id_, SHUT_WR);
      em_->disableWriteEvent(id_);
      em_->registerReadEvent(id_);
    }
  }
}
