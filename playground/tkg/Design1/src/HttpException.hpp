#ifndef HTTP_EXCEPTION_HPP_
#define HTTP_EXCEPTION_HPP_
#include <stdexcept>

#include "../const.hpp"

class HttpException : public std::runtime_error {
 public:
  HttpException(int statusCode, const std::string &statusMessage)
      : std::runtime_error(statusMessage), statusCode_(statusCode) {}

  int statusCode() const { return statusCode_; }

 private:
  int statusCode_;
};

// 4xx status code
class BadRequestException : public HttpException {
 public:
  BadRequestException(const std::string &message) : HttpException(400, message) {}
};

class ResourceForbidenException : public HttpException {
 public:
  ResourceForbidenException(const std::string &message) : HttpException(403, message) {}
};

class ResourceNotFoundException : public HttpException {
 public:
  ResourceNotFoundException(const std::string &message) : HttpException(404, message) {}
};

class NotAllowedException : public HttpException {
 public:
  NotAllowedException(const std::string &message) : HttpException(405, message) {}
};

// 5xx error status code
class InternalServerErrorException : public HttpException {
 public:
  InternalServerErrorException(const std::string &message) : HttpException(500, message) {}
};

class NotImplementedException : public HttpException {
 public:
  NotImplementedException(const std::string &message) : HttpException(501, message) {}
};

class VersionNotSupportedException : public HttpException {
 public:
  VersionNotSupportedException(const std::string &message) : HttpException(505, message) {}
};

#endif