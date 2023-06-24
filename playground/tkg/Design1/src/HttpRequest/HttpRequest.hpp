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
  HttpRequest(const HttpRequest &other)
      : method_(other.method_), version_(other.version_), headers_(other.headers_), body_(other.body_) {
    if (other.request_target_ != NULL) {
      this->request_target_ = new URI(*other.request_target_);
    } else {
      this->request_target_ = NULL;
    }
  }
  ~HttpRequest() { delete request_target_; };

  void swap(HttpRequest &lhs, HttpRequest &rhs) {
    using std::swap;
    swap(lhs.method_, rhs.method_);
    swap(lhs.version_, rhs.version_);
    swap(lhs.request_target_, rhs.request_target_);
    swap(lhs.headers_, rhs.headers_);
    swap(lhs.body_, rhs.body_);
  }

  // this will invoke the copy constructor, and then destructor for the old data
  HttpRequest &operator=(HttpRequest other) {
    swap(*this, other);
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
