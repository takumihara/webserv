#include "HttpResponse.hpp"

#include <stdio.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <sstream>

#include "EventManager.hpp"
#include "const.hpp"

int HttpResponse::getStatus() const { return status_; };
void HttpResponse::setStatus(const int status) { status_ = status; }

void HttpResponse::appendBody(const std::string &str) { body_ += str; }

void HttpResponse::appendHeader(const std::string &key, const std::string &value) {
  headers.push_back(header(key, value));
}

const std::string &HttpResponse::getBody() const { return body_; }

void HttpResponse::createResponse() {
  if (state_ != Free) return;
  std::stringstream ss;
  ss << body_.size();
  appendHeader("Content-Length", ss.str());
  ss.str("");
  ss.clear(std::stringstream::goodbit);
  // stus-line
  ss << "HTTP/1.1 " << status_ << " " << conf_->cache_.statusMsg_[status_] << CRLF;
  // header-fields
  for (std::vector<header>::const_iterator itr = headers.cbegin(); itr != headers.cend(); itr++) {
    ss << itr->first << ": " << itr->second << CRLF;
  }
  ss << CRLF;
  // body
  ss << body_ << CRLF;

  response_ = ss.str();
  std::cout << response_;
  response_size_ = response_.size();
  sending_response_size_ = 0;
  state_ = Sending;
}

void HttpResponse::sendResponse() {
  DEBUG_PUTS("sending response");
  if (state_ != Sending) return;
  const char *response = response_.c_str();
  int size = response_size_ - sending_response_size_;
  if (size > SOCKET_WRITE_SIZE) {
    size = SOCKET_WRITE_SIZE;
  }
  int res = sendto(sock_fd_, &response[sending_response_size_], size, 0, NULL, 0);
  if (res == -1) {
    perror("sendto");
    throw std::runtime_error("send error");
  }
  std::string res_str = std::string(response);
  std::cout << "response sent: "
            << "'" << res_str.substr(sending_response_size_, size) << "'"
            << " (size:" << res << ")" << std::endl;
  sending_response_size_ += size;
  std::cout << "response size: " << response_size_ << "(" << sending_response_size_ << std::endl;
  if (sending_response_size_ == response_size_) state_ = End;
}

void HttpResponse::refresh() {
  (void)port_;
  state_ = Free;
  status_ = 0;
  body_.clear();
  response_.clear();
  headers.clear();
  sending_response_size_ = 0;
  response_size_ = 0;
}
