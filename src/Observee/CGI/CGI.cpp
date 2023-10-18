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

void CGI::timeout() {
  DEBUG_PUTS("CGI::timeout");
  response_ = dynamic_cast<ConnectionSocket *>(parent_)->initResponse();
  response_->setStatusAndReason(500);
  em_->enableTimerEvent(parent_->id_);
  em_->registerWriteEvent(parent_->id_);
  parent_->stopMonitorChild(this);
  parent_ = NULL;
  // for (std::vector<Observee *>::iterator itr = children_.begin(); itr != children_.end(); itr++) {
  //   (*itr)->parent_ = NULL;
  //   (*itr)->shutdown();
  // }
  // children_.clear();
  close(id_);
  em_->deleteTimerEvent(id_);
  kill(pid_, SIGTERM);
  waitpid(pid_, NULL, 0);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void CGI::shutdown() {
  DEBUG_PUTS("CGI shutdown");
  parent_->stopMonitorChild(this);
  parent_ = NULL;
  // for (std::vector<Observee *>::iterator itr = children_.begin(); itr != children_.end(); itr++) {
  //   (*itr)->parent_ = NULL;
  //   (*itr)->shutdown();
  // }
  // children_.clear();
  int status = 0;
  close(id_);
  em_->deleteTimerEvent(id_);
  kill(pid_, SIGTERM);
  waitpid(pid_, &status, 0);
  if (status != 0) response_->setStatusAndReason(500);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void CGI::terminate() {
  close(id_);
  kill(pid_, SIGTERM);
  waitpid(pid_, NULL, 0);
}

std::pair<std::string, std::string> getHeaderField(std::string &field) {
  std::stringstream ss(field);
  std::string name, value;
  if (field.find(':') == std::string::npos) throw InternalServerErrorException("invalid header field");
  std::getline(ss, name, ':');
  toLower(name);
  std::getline(ss, value);
  value = trimOws(value);
  return std::pair<std::string, std::string>(name, value);
}

void CGI::processDocRes(std::string &body) {
  DEBUG_PUTS("processDocRes");
  for (HttpResponse::t_headers::const_iterator itr = headers_.cbegin(); itr != headers_.cend(); itr++) {
    if (itr->first == "status") {
      std::istringstream iss(itr->second);
      std::string status;
      std::string reason;
      iss >> status;
      iss.ignore();
      getline(iss, reason);
      response_->setStatusAndReason(std::atoi(status.c_str()), reason);
    } else {
      response_->appendHeader(itr->first, itr->second);
    }
  }
  response_->appendBody(body);
  em_->registerWriteEvent(parent_->id_);
  em_->enableTimerEvent(parent_->id_);
  shutdown();
}

void CGI::processClientRedirect() {
  DEBUG_PUTS("processClientRedirect");
  for (HttpResponse::t_headers::const_iterator itr = headers_.cbegin(); itr != headers_.cend(); itr++) {
    response_->appendHeader(itr->first, itr->second);
  }
  response_->setStatusAndReason(302);
  em_->registerWriteEvent(parent_->id_);
  em_->enableTimerEvent(parent_->id_);
  shutdown();
}

void CGI::processClientRedirectWithDoc(std::string &body) {
  DEBUG_PUTS("processClientRedirectWithDoc");
  for (HttpResponse::t_headers::const_iterator itr = headers_.cbegin(); itr != headers_.cend(); itr++) {
    if (itr->first == "status") {
      std::istringstream iss(itr->second);
      std::string status;
      std::string reason;
      iss >> status;
      iss.ignore();
      getline(iss, reason);
      response_->setStatusAndReason(std::atoi(status.c_str()), reason);
    } else {
      response_->appendHeader(itr->first, itr->second);
    }
  }
  response_->appendBody(body);
  em_->registerWriteEvent(parent_->id_);
  em_->enableTimerEvent(parent_->id_);
  shutdown();
}

void CGI::processLocalRedirect() {
  DEBUG_PUTS("processLocalRedirect");
  std::string loc_value = headers_["location"];
  try {
    const size_t fragment_pos = loc_value.find("#");
    std::string fragment;
    if (fragment_pos != std::string::npos) {
      fragment = loc_value.substr(fragment_pos + 1);
    }
    URI *uri = URI::parseRequestURI(loc_value.substr(0, fragment_pos));
    delete request_->request_target_;
    request_->request_target_ = uri;
    request_->body_.clear();
    dynamic_cast<ConnectionSocket *>(parent_)->initExtension();
    ConnectionSocket *parent = dynamic_cast<ConnectionSocket *>(parent_);
    shutdown();
    parent->process();
  } catch (std::runtime_error &e) {
    throw InternalServerErrorException("CGI Local Redirect response URI is invalid");
  }
}

bool CGI::isDocRes() {
  if (headers_.find("content-type") == headers_.end()) return false;
  if (!CGIValidation::isMediaType(headers_["content-type"])) return false;
  if (headers_.find("status") != headers_.end() && !CGIValidation::isStatusHeaderValue(headers_["status"]))
    return false;
  if (headers_.find("location") != headers_.end()) return false;
  return true;
}

bool CGI::isLocalRedirectRes() {
  if (headers_.size() != 1) return false;
  if (headers_.find("location") == headers_.end()) return false;
  if (!CGIValidation::isAbsPath(headers_["location"].c_str())) return false;

  return true;
}

bool CGI::isClientRedirectRes() {
  if (headers_.find("location") == headers_.end()) return false;
  if (!CGIValidation::isAbsURI(headers_["location"])) return false;
  if (headers_.find("content-type") != headers_.end()) return false;
  if (headers_.find("status") != headers_.end()) return false;
  return true;
}

bool CGI::isClientRedirectWithDocRes() {
  if (headers_.find("location") == headers_.end()) return false;
  if (!CGIValidation::isAbsURI(headers_["location"])) return false;
  if (headers_.find("content-type") == headers_.end()) return false;
  if (!CGIValidation::isMediaType(headers_["content-type"])) return false;
  if (headers_.find("status") != headers_.end()) {
    if (!CGIValidation::isStatusHeaderValue(headers_["status"])) return false;
  }
  return true;
}

CGI::Type CGI::getResponseType() {
  if (isDocRes()) {
    return CGI::Doc;
  } else if (isLocalRedirectRes()) {
    return CGI::LocalRedir;
  } else if (isClientRedirectRes()) {
    return CGI::ClientRedir;
  } else if (isClientRedirectWithDocRes()) {
    return CGI::ClientRedirWithDoc;
  }
  return CGI::Error;
}

void CGI::parseHeaders(std::string &headers) {
  std::vector<std::string> fields = CGIValidation::extractLines(headers);
  std::vector<std::string>::size_type i = 0;
  while (i < fields.size() && fields[i] != "") {
    t_field field = getHeaderField(fields[i]);
    headers_[field.first] = field.second;
    i++;
  }
}

void CGI::handleCGIResponse() {
  DEBUG_PRINTF("CGI LAST RESULT: '%s'\n", escape(recieved_data_).c_str());
  std::size_t nlnl_pos = recieved_data_.find("\n\n");
  if (nlnl_pos == std::string::npos)
    throw InternalServerErrorException("CGI response is invalid(no end NL of headers)");
  std::string headers = recieved_data_.substr(0, nlnl_pos);
  std::string body = recieved_data_.substr(recieved_data_.find("\n\n") + 2);
  parseHeaders(headers);
  CGI::Type type = getResponseType();
  if (type == CGI::Error) {
    throw InternalServerErrorException("CGI response is invalid");
  } else if (type == CGI::Doc) {
    processDocRes(body);
  } else if (type == CGI::LocalRedir) {
    processLocalRedirect();
  } else if (type == CGI::ClientRedir) {
    processClientRedirect();
  } else if (type == CGI::ClientRedirWithDoc) {
    processClientRedirectWithDoc(body);
  }
}

void CGI::notify(struct kevent ev) {
  DEBUG_PUTS("handle CGI");
  (void)recieved_size_;

  if (ev.filter == EVFILT_READ) {
    char buff[FILE_READ_SIZE + 1];

    int res = read(id_, &buff[0], FILE_READ_SIZE);
    if (res == -1) {
      response_->setStatusAndReason(500);
      em_->registerWriteEvent(parent_->id_);
      em_->enableTimerEvent(parent_->id_);
      shutdown();
      return;
    } else if (res == 0) {
      response_->setStatusAndReason(200);
      try {
        handleCGIResponse();
      } catch (HttpException &e) {
        DEBUG_PUTS(e.what());
        response_->setStatusAndReason(e.statusCode());
        em_->registerWriteEvent(parent_->id_);
        em_->enableTimerEvent(parent_->id_);
        shutdown();
      }
    } else {
      DEBUG_PRINTF("CGI read res: %d\n", res);
      buff[res] = '\0';
      recieved_data_ += buff;
      DEBUG_PRINTF("CGI WIP RESULT: '%s'\n", escape(recieved_data_).c_str());
    }
  } else if (ev.filter == EVFILT_WRITE) {
    const char *response = request_->body_.data();
    DEBUG_PRINTF("response: %s\n", escape(std::string(request_->body_.begin(), request_->body_.end())).c_str());
    std::size_t size = request_->body_.size() - sending_size_;

    if (size > SOCKET_WRITE_SIZE) {
      size = SOCKET_WRITE_SIZE;
    }
    int res = write(id_, response + sending_size_, size);
    if (res == -1) {
      perror("sendto");
      response_->setStatusAndReason(500);
      em_->registerWriteEvent(parent_->id_);
      em_->enableTimerEvent(parent_->id_);
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
