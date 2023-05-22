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

std::string escape(const std::string &str) {
  std::string res;
  for (size_t i = 0; i < str.size(); i++) {
    switch (str[i]) {
      case '\n':
        res += "\\n";
        break;
      case '\r':
        res += "\\r";
        break;
      case '\t':
        res += "\\t";
        break;
      default:
        res += str[i];
    }
  }
  return res;
}
