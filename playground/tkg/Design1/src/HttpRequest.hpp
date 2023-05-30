#ifndef HTTP_REQUEST_HPP_
#define HTTP_REQUEST_HPP_
#include <sys/event.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <set>
#include <string>

#include "./Config/Config.hpp"

class EventManager;

class HttpRequest {
 public:
  enum State { ReadingStartLine, ReadingHeaders, ReadingChunkedBody, ReadingBody, End };
  enum ReadingChunkedState { ReadingChunkedSize, ReadingChunkedData };
  enum RequestTargetType { OriginForm, AbsoluteForm, AuthorityForm, AsteriskForm };
  enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };
  enum Version { HTTP1_1 };
  enum HeaderField { HostField, ContentLengthField, TransferEncodingField, DateField };
  enum TransferEncoding { Chunked, Compress, Deflate, Gzip };
  struct RequestTarget {
    RequestTargetType type;
    // origin form
    std::string absolute_path;
    std::string query;
  };
  struct RequestLine {
    Method method;
    RequestTarget request_target;
    Version version;
  };
  struct Host {
    std::string uri_host;
    int port;
  };
  struct Headers {
    Host host;
    size_t content_length;
    std::vector<TransferEncoding> transfer_encodings;
    std::tm date;
  };

  HttpRequest(int fd, int port, Config &conf)
      : sock_fd_(fd),
        port_(port),
        state_(ReadingStartLine),
        chunked_size_(0),
        chunked_reading_state_(ReadingChunkedSize),
        conf_(conf) {}
  ~HttpRequest(){};
  HttpRequest &operator=(const HttpRequest &other) {
    if (this != &other) {
      this->sock_fd_ = other.sock_fd_;
      this->port_ = other.port_;
      this->raw_data_ = other.raw_data_;
      this->rest_ = other.rest_;
      this->state_ = other.state_;
      this->request_line_ = other.request_line_;
      this->headers_ = other.headers_;
      this->received_fields_ = other.received_fields_;
      this->body_ = other.body_;
      this->chunked_size_ = other.chunked_size_;
      this->chunked_reading_state_ = other.chunked_reading_state_;
      this->conf_ = other.conf_;
    }
    return *this;
  }
  bool readRequest(EventManager &em);
  void refresh();
  const std::string &getBody() const;
  const Host &getHost() const;
  const RequestTarget &getRequestTarget() const;
  bool isChunked();

  class BadRequestException : public std::exception {
    virtual const char *what() const throw() { return "Bad Request"; };
  };
  class NotImplementedException : public std::exception {
    virtual const char *what() const throw() { return "Not Implemented"; };
  };
  class NotAllowedException : public std::exception {
    virtual const char *what() const throw() { return "Method Not Allowed"; };
  };
  class VersionNotSupportedException : public std::exception {
    virtual const char *what() const throw() { return "HTTP Version Not Supported"; };
  };

 private:
  int sock_fd_;
  int port_;
  std::string raw_data_;
  std::string rest_;
  State state_;

  RequestLine request_line_;
  Headers headers_;
  std::set<HeaderField> received_fields_;
  std::string body_;
  size_t chunked_size_;
  ReadingChunkedState chunked_reading_state_;
  Config &conf_;
  const static char *kSupportedTransferEncodings[];

  std::string getEndingChars() const;
  void trimToEndingChars();
  void moveToNextState();

  void parseStartline();

  void assignAndValidateMethod(const std::string &method);
  void assignAndValidateRequestTarget(const std::string &requestTarget);
  void assignAndValidateVersion(const std::string &version);

  void parseHeaders();
  void validateHeaderName(const std::string &name);
  void validateHeaderValue(const std::string &value);
  void validateHeaders();
  void insertIfNotDuplicate(HeaderField field, const char *error_msg);

  void initAnalyzeFuncs(std::map<std::string, void (HttpRequest::*)(const std::string &)> &analyze_funcs);
  void analyzeHost(const std::string &value);
  void analyzeContentLength(const std::string &value);
  void analyzeTransferEncoding(const std::string &value);
  void analyzeDate(const std::string &value);
  void analyzeServer(const std::string &value);

  void readBody();
  bool readChunkedBody();

  bool isReceivingBody();
  bool isActionable();

  void printHeaders();
};

#endif
