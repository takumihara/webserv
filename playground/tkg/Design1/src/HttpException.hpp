#ifndef HTTP_EXCEPTION_HPP_
#define HTTP_EXCEPTION_HPP_
#include <stdexcept>


class HttpException : public std::runtime_error {
 public:
  HttpException(int statusCode, const std::string &statusMessage)
      : std::runtime_error(statusMessage), statusCode_(statusCode) {}

  int statusCode() const { return statusCode_; }

 private:
  int statusCode_;
};


// 3xx status code
class RedirectMovedPermanently : public HttpException {
 public:
  RedirectMovedPermanently(const std::string &message) : HttpException(301, message) {}
};

class RedirectSeeOther : public HttpException {
 public:
  RedirectSeeOther(const std::string &message) : HttpException(303, message) {}
};

class RedirectTemporaryRedirect : public HttpException {
 public:
  RedirectTemporaryRedirect(const std::string &message) : HttpException(307, message) {}
};

class RedirectPermanentRedirect : public HttpException {
 public:
  RedirectPermanentRedirect(const std::string &message) : HttpException(308, message) {}
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