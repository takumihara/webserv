#include "HttpResponse.hpp"

#include <stdio.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <sstream>
#include <string>

#include "Config/validation.h"
#include "EventManager.hpp"
#include "const.hpp"
#include "helper.hpp"

int HttpResponse::getStatus() const { return status_; };
void HttpResponse::setStatus(const int status) { status_ = status; }
void HttpResponse::setStatusAndReason(const int status, const std::string &reason) {
  status_ = status;
  if (reason == "")
    reason_phrase_ = conf_->cache_.statusMsg_[status_];
  else
    reason_phrase_ = reason;
}

void HttpResponse::setContentType(const std::string &path) {
  std::string ext = getExtension(path);
  if (ext == "") return;
  if (hasHeader("content-type")) return;
  appendHeader("content-type", conf_->cache_.ext_contentType_map_[ext]);
}

void HttpResponse::appendBody(const char *str, size_t size) { body_.insert(body_.end(), str, str + size); }

void HttpResponse::appendBody(const std::string &str) { body_.insert(body_.end(), str.begin(), str.end()); }

void HttpResponse::appendHeader(const std::string &key, const std::string &value) { headers_[key] = value; }

const std::vector<char> &HttpResponse::getBody() const { return body_; }

bool HttpResponse::hasHeader(const std::string &target_name) {
  t_headers::iterator key = headers_.find(target_name);
  if (key == headers_.end()) return false;
  return true;
}

void HttpResponse::createResponse() {
  if (state_ != Free) return;
  if (!hasHeader("content-length")) {
    std::stringstream ss;
    ss << body_.size();
    appendHeader("content-length", ss.str());
  }
  std::stringstream ss;
  // status-line
  ss << "HTTP/1.1 " << status_ << " " << conf_->cache_.statusMsg_[status_] << CRLF;
  // header-fields
  for (t_headers::const_iterator itr = headers_.cbegin(); itr != headers_.cend(); itr++) {
    ss << itr->first << ": " << itr->second << CRLF;
  }
  ss << CRLF;
  std::string head = ss.str();
  response_.insert(response_.end(), head.begin(), head.end());
  response_.insert(response_.end(), body_.begin(), body_.end());
  std::cout << "full response: '" << escape(std::string(&response_[0], response_.size())) << "'" << std::endl;
  response_size_ = response_.size();
  sending_response_size_ = 0;
  state_ = Sending;
}

void HttpResponse::sendResponse() {
  DEBUG_PUTS("sending response");
  if (state_ != Sending) return;
  const char *response = &response_[0];
  int size = response_size_ - sending_response_size_;
  if (size > SOCKET_WRITE_SIZE) {
    size = SOCKET_WRITE_SIZE;
  }
  int res = sendto(sock_fd_, &response[sending_response_size_], size, 0, NULL, 0);
  if (res == -1) {
    perror("sendto");
    throw std::runtime_error("send error");
  }
  std::string res_str = std::string(response, response_.size());
  std::cout << "response sent: "
            << "'" << escape(res_str.substr(sending_response_size_, size)) << "'"
            << " (size:" << res << ")" << std::endl;
  sending_response_size_ += size;
  std::cout << "response size: " << response_size_ << "(" << sending_response_size_ << ")" << std::endl;
  if (sending_response_size_ == response_size_) state_ = End;
}

void HttpResponse::refresh() {
  (void)port_;
  state_ = Free;
  status_ = 0;
  body_.clear();
  response_.clear();
  headers_.clear();
  sending_response_size_ = 0;
  response_size_ = 0;
}
