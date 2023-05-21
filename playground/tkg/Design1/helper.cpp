#include "helper.hpp"

#include <algorithm>

bool in(const std::string &str, const std::string *arr, size_t size) {
  return std::find(arr, arr + size, str) != arr + size;
}

char asciitolower(char in) {
  if (in <= 'Z' && in >= 'A') return in - ('Z' - 'z');
  return in;
}

void toLower(std::string &str) { std::transform(str.begin(), str.end(), str.begin(), asciitolower); }
