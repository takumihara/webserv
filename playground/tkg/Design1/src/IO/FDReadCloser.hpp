#pragma once

#include "IReadCloser.hpp"

// FDReadCloser doesn't close its fd automatically. You have to call close() explicitly.
class FDReadCloser : public IReadCloser {
 public:
  FDReadCloser(int fd) : fd_(fd) {}
  ~FDReadCloser() {}
  // read may throw(std::runtime_error)
  size_t read(std::string &buf, size_t size);
  void close();

 private:
  int fd_;
};
