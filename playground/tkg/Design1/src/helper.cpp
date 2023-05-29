#include "helper.hpp"

#include <algorithm>

#include "const.hpp"

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

std::string trimOws(const std::string &str) {
  std::string::size_type start = str.find_first_not_of(OWS);
  std::string::size_type end = str.find_last_not_of(OWS);
  if (start == std::string::npos) return "";
  return str.substr(start, end - start + 1);
}

std::string trimUntilCRLF(const std::string &str) { return str.substr(0, str.find(CRLF)); }
