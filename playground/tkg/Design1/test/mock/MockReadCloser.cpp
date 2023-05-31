#include "MockReadCloser.hpp"

#include <unistd.h>

#include <iostream>

size_t MockReadCloser::read(std::string &buf, size_t size) {
  if (size < content_.size()) {
    size = content_.size();
    buf = content_.substr(0, size);
    content_ = content_.substr(size);
    return size;
  }
  buf = content_;
  content_ = "";
  return buf.size();
}
void MockReadCloser::close() {}
