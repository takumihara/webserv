#pragma once

#include <sys/event.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <set>
#include <string>

#include "./Config/Config.hpp"
#include "./IO/IReadCloser.hpp"
#include "./URI/URI.hpp"

class HttpRequest {
 public:
  enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };
  enum Version { HTTP1_1 };
  enum TransferEncoding { Chunked, Compress, Deflate, Gzip };
  struct Host {
    std::string uri_host;
    int port;
  };
  class Headers {
   public:
    Headers() : content_length(0) {}
    Host host;
    size_t content_length;
    std::string content_type;
    std::vector<TransferEncoding> transfer_encodings;
    std::tm date;
  };

  HttpRequest() : request_target_(NULL) {}
  ~HttpRequest() { delete request_target_; };
  HttpRequest &operator=(const HttpRequest &other) {
    if (this != &other) {
      this->method_ = other.method_;
      this->version_ = other.version_;
      this->request_target_ = other.request_target_;
      this->headers_ = other.headers_;
      this->body_ = other.body_;
    }
    return *this;
  }
  bool methodIs(Method method) const;
  bool isChunked();
  void printHeaders();

  Method method_;
  Version version_;
  URI *request_target_;
  Headers headers_;
  std::string body_;

 private:
};
