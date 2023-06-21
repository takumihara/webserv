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
#include "HttpRequest.hpp"

class HttpRequestReader {
 public:
  enum State { ReadingStartLine, ReadingHeaders, ReadingChunkedBody, ReadingBody, FinishedReading, SocketClosed, End };
  enum ReadingChunkedState { ReadingChunkedSize, ReadingChunkedData };
  enum HeaderField { HostField, ContentLengthField, TransferEncodingField, DateField };

  HttpRequestReader(int fd, Config *conf, HttpRequest &request, IReadCloser *rc, const std::string &remaining_data = "")
      : request_(request),
        rc_(rc),
        sock_fd_(fd),
        raw_data_(remaining_data),
        state_(ReadingStartLine),
        chunked_size_(0),
        chunked_reading_state_(ReadingChunkedSize),
        conf_(conf) {}
  ~HttpRequestReader(){};
  HttpRequestReader &operator=(const HttpRequestReader &other) {
    if (this != &other) {
      this->sock_fd_ = other.sock_fd_;
      this->raw_data_ = other.raw_data_;
      this->rest_ = other.rest_;
      this->state_ = other.state_;
      this->chunked_size_ = other.chunked_size_;
      this->chunked_reading_state_ = other.chunked_reading_state_;
      this->conf_ = other.conf_;
    }
    return *this;
  }
  State read();

 private:
  std::set<HeaderField> received_fields_;

  HttpRequest &request_;
  IReadCloser *rc_;

  int sock_fd_;
  std::string raw_data_;
  std::string rest_;
  State state_;
  size_t chunked_size_;
  ReadingChunkedState chunked_reading_state_;
  Config *conf_;
  const static char *kSupportedTransferEncodings[];

  std::string getEndingChars() const;
  void trimToEndingChars();
  void moveToNextState();

  bool hasField(HeaderField field) const;
  void insertIfNotDuplicate(HeaderField field, const char *error_msg);

  void parseStartline();

  void assignAndValidateMethod(const std::string &method);
  void assignAndValidateRequestTarget(const std::string &requestTarget);
  void assignAndValidateVersion(const std::string &version);

  void parseHeaders();
  void validateHeaderName(const std::string &name);
  void validateHeaderValue(const std::string &value);
  void validateHeaders();

  void initAnalyzeFuncs(std::map<std::string, void (HttpRequestReader::*)(const std::string &)> &analyze_funcs);
  void analyzeHost(const std::string &value);
  void analyzeContentLength(const std::string &value);
  void analyzeTransferEncoding(const std::string &value);
  void analyzeDate(const std::string &value);
  void analyzeServer(const std::string &value);

  void readBody();
  void readChunkedBody();

  bool isReceivingBody();
  bool isActionable();
};
