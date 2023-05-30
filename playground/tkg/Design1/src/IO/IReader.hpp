#pragma once

#include <string>

class IReader {
 public:
  // it reads up to buf.size() bytes and stores them in buf. It returns the number of bytes read.
  virtual size_t read(std::string &buf, size_t size) = 0;
  virtual ~IReader() {}
};
