#include "helper.hpp"

#include <cctype>

namespace CGI {

bool isMark(char c) {
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

bool isUnreserved(char c) { return std::isalnum(c) || isMark(c); }

bool isHex(char c) {
  if ('0' <= c && c <= '9')
    return true;
  else if ('a' <= c && c <= 'f')
    return true;
  else if ('A' <= c && c <= 'F')
    return true;
  return false;
}

bool isEscaped(char *escape) { return escape[0] == '%' && isHex(escape[1]) && isHex(escape[2]); }

bool isReserved(char c) {
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

bool isAbsPath(char *path) {
  if (*path != '/') return false;
  path++;
  bool pre_is_slash = true;
  while (*path != '\0') {
    if (*path == '/') {
      if (pre_is_slash == true) return false;
      pre_is_slash = true;
    } else if (isPchar(path)) {
      if (*path == '%') path += 2;
    } else {
      return false;
    }
    path++;
  }
  return true;
}

bool isPchar(char *c) {
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

};  // namespace CGI