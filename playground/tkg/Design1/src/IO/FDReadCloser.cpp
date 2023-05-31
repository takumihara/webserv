#include "FDReadCloser.hpp"

#include <unistd.h>

#include <iostream>

size_t FDReadCloser::read(std::string &buf, size_t size) {
  char array[size + 1];
  bzero(array, size + 1);
  std::cout << size << std::endl;
  ssize_t res = ::read(fd_, array, size);
  if (res == -1) {
    throw std::runtime_error("read failed");
  }
  array[res] = '\0';
  buf = array;
  return res;
}
void FDReadCloser::close() { ::close(fd_); }
