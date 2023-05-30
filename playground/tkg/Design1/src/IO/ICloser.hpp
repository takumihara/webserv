#pragma once

class ICloser {
 public:
  virtual void close() = 0;
  virtual ~ICloser() {}
};
