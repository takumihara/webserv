#pragma once

#include "../../src/IO/IReadCloser.hpp"

class MockReadCloser : public IReadCloser {
 public:
  MockReadCloser(const std::string &content) : content_(content) {}
  ~MockReadCloser() {}
  size_t read(std::string &buf, size_t size);
  void close();

 private:
  std::string content_;
};
