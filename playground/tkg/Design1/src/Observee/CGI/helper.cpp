#include "helper.hpp"

#include <cctype>
#include <sstream>

#include "../../URI/URI.hpp"

namespace CGIValidation {

bool isMark(const char c) {
  switch (c) {
    case '-':
    case '_':
    case '.':
    case '!':
    case '~':
    case '*':
    case '\'':
    case '(':
    case ')':
      return true;
    default:
      return false;
  }
}

bool isUnreserved(const char c) { return std::isalnum(c) || isMark(c); }

bool isHex(const char c) {
  if ('0' <= c && c <= '9')
    return true;
  else if ('a' <= c && c <= 'f')
    return true;
  else if ('A' <= c && c <= 'F')
    return true;
  return false;
}

bool isEscaped(const char *escape) { return escape[0] == '%' && isHex(escape[1]) && isHex(escape[2]); }

bool isReserved(const char c) {
  switch (c) {
    case ';':
    case '/':
    case '?':
    case ':':
    case '@':
    case '&':
    case '=':
    case '+':
    case '$':
    case ',':
    case '[':
    case ']':
      return true;
    default:
      return false;
  }
}

bool isPchar(const char *c) {
  switch (*c) {
    case ':':
    case '@':
    case '&':
    case '=':
    case '+':
    case '$':
    case ',':
      return true;
  }
  return isUnreserved(*c) || isEscaped(c);
}

std::vector<std::string> extractLines(const std::string &data) {
  std::vector<std::string> lines;
  std::string str;
  std::stringstream ss(data);
  while (getline(ss, str)) {
    lines.push_back(str);
  }
  return lines;
}

bool isAbsPath(const char *path) {
  if (*path != '/') return false;
  path++;
  while (*path != '\0') {
    if (*path == '/' || isPchar(path)) {
      if (*path == '%') path += 2;
    } else {
      return false;
    }
    path++;
  }
  return true;
}

bool isAbsURI(const std::string &raw_uri) {
  // todo:
  try {
    URI *uri = URI::parse(raw_uri);
    if (uri->getScheme() == "") return false;
    delete uri;
  } catch (std::runtime_error &e) {
    return false;
  }
  return true;
}

};  // namespace CGIValidation