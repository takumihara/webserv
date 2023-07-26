#include <cctype>

bool isUnreserved(char c) { return std::isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~'; }

bool isGenDelims(char c) {
  switch (c) {
    case ':':
    case '/':
    case '?':
    case '#':
    case '[':
    case ']':
    case '@':
      return true;
    default:
      return false;
  }
}

bool isSubDelims(char c) {
  switch (c) {
    case '!':
    case '$':
    case '&':
    case '\'':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case ';':
    case '=':
      return true;
    default:
      return false;
  }
}

bool isReserved(char c) { return isGenDelims(c) || isSubDelims(c); }
