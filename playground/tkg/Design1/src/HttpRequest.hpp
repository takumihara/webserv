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
#include "./IO/IReadCloser.hpp"
#include "./URI/URI.hpp"

class EventManager;

class HttpRequest {
 public:
  enum State { ReadingStartLine, ReadingHeaders, ReadingChunkedBody, ReadingBody, FinishedReading, SocketClosed, End };
  enum ReadingChunkedState { ReadingChunkedSize, ReadingChunkedData };
  enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };
  enum Version { HTTP1_1 };
  enum HeaderField { HostField, ContentLengthField, TransferEncodingField, DateField };
  enum TransferEncoding { Chunked, Compress, Deflate, Gzip };
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

  HttpRequest(int fd, Config *conf)
      : sock_fd_(fd),
        state_(ReadingStartLine),
        chunked_size_(0),
        chunked_reading_state_(ReadingChunkedSize),
        conf_(conf),
        request_target_(NULL) {}
  ~HttpRequest() { delete request_target_; };
  HttpRequest &operator=(const HttpRequest &other) {
    if (this != &other) {
      this->sock_fd_ = other.sock_fd_;
      this->raw_data_ = other.raw_data_;
      this->rest_ = other.rest_;
      this->state_ = other.state_;
      this->method_ = other.method_;
      this->version_ = other.version_;
      this->request_target_ = other.request_target_;
      this->headers_ = other.headers_;
      this->received_fields_ = other.received_fields_;
      this->body_ = other.body_;
      this->chunked_size_ = other.chunked_size_;
      this->chunked_reading_state_ = other.chunked_reading_state_;
      this->conf_ = other.conf_;
    }
    return *this;
  }
  void refresh();
  const std::string &getBody() const;
  const Host &getHost() const;
  bool methodIs(Method method) const;
  URI *getRequestTarget() const;
  const Method &getMethod() const;
  bool isChunked();

  static State readRequest(HttpRequest &req, IReadCloser *rc);

 private:
  int sock_fd_;
  std::string raw_data_;
  std::string rest_;
  State state_;
  size_t chunked_size_;
  ReadingChunkedState chunked_reading_state_;

  std::set<HeaderField> received_fields_;
  Config *conf_;
  const static char *kSupportedTransferEncodings[];

  Method method_;
  Version version_;
  URI *request_target_;
  Headers headers_;
  std::string body_;

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
  bool hasField(HeaderField field) const;
  void insertIfNotDuplicate(HeaderField field, const char *error_msg);

  void initAnalyzeFuncs(std::map<std::string, void (HttpRequest::*)(const std::string &)> &analyze_funcs);
  void analyzeHost(const std::string &value);
  void analyzeContentLength(const std::string &value);
  void analyzeTransferEncoding(const std::string &value);
  void analyzeDate(const std::string &value);
  void analyzeServer(const std::string &value);

  void readBody();
  void readChunkedBody();

  bool isReceivingBody();
  bool isActionable();

  void printHeaders();
};

#endif
