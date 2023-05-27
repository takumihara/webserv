#ifndef HTTP_REQUEST_HPP_
#define HTTP_REQUEST_HPP_
#include <sys/event.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>

#include "./Config/Config.hpp"

#define SP ' '
#define CRLF "\r\n"
#define OWS " \t"

class EventManager;

class HttpRequest {
 public:
  enum State { ReadingStartLine, ReadingHeaders, ReadingChunkedBody, ReadingBody, End };
  enum ReadingChunkedState { ReadingChunkedSize, ReadingChunkedData };
  enum RequestTargetType { OriginForm, AbsoluteForm, AuthorityForm, AsteriskForm };
  enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };
  enum Version { HTTP1_1 };
  enum TransferEncoding { Chunked, Compress, Deflate, Gzip };
  struct RequestTarget {
    RequestTargetType type;
    // origin form
    std::string absolutePath;
    std::string query;
  };
  struct RequestLine {
    Method method;
    RequestTarget requestTarget;
    Version version;
  };
  struct Host {
    std::string uri_host;
    int port;
  };
  struct Headers {
    Host host;
    size_t content_length;
    std::vector<TransferEncoding> transferEncodings;
    std::tm date;
  };

  HttpRequest(int fd, int port)
      : sock_fd_(fd),
        port_(port),
        state_(ReadingStartLine),
        chunked_size_(0),
        chunked_reading_state_(ReadingChunkedSize) {}
  ~HttpRequest(){};
  bool readRequest(EventManager &em);
  void refresh();
  const std::string &getBody() const;
  const Host &getHost() const;
  const RequestTarget &getRequestTarget() const;
  bool isChunked();

  // private:
  int sock_fd_;
  int port_;
  // Config &conf_;
  std::string raw_data_;
  std::string rest_;
  State state_;

  RequestLine request_line_;
  Headers headers_;
  std::string body_;
  size_t chunked_size_;
  ReadingChunkedState chunked_reading_state_;

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
  void analyzeHost(const std::string &value);
  void analyzeContentLength(const std::string &value);
  void analyzeTransferEncoding(const std::string &value);
  void analyzeDate(const std::string &value);
  void analyzeServer(const std::string &value);

  void readBody();
  bool readChunkedBody();

  bool isReceivingBody();
  bool isActionable();
};

#endif
