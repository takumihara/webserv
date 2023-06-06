#include "encoding.hpp"

#include <cctype>
#include <string>

#include "helper.hpp"

const char* upperhex = "0123456789ABCDEF";

bool shouldEscape(char c, Encoding::Type mode) {
  if (isUnreserved(c) || isSubDelims(c) || c == '%') {
    return false;
  }

  switch (mode) {
    case Encoding::Host:
      // we add : because we include :port as part of host
      // I'm not so sure about this, but RFC doesn't allow to use %-encoding for ascii bytes
      if (c == ':' || isReserved(c) || c == ' ') {
        return false;
      }
      break;
    case Encoding::Path:
      if (c == '/' || c == ':' || c == '@') {
        return false;
      }
      break;
    case Encoding::PathSegment:
      if (c == ':' || c == '@') {
        return false;
      }
      break;
    case Encoding::UserInfo:
      if (c == ':') {
        return false;
      }
      break;
    case Encoding::QueryComponent:
    case Encoding::Fragment:
      if (c == ':' || c == '@' || c == '/' || c == '?') {
        return false;
      }
    default:
      return true;
  }

  return true;
}

std::string Encoding::escape(const std::string& str, Type mode) {
  int space_count = 0;
  int hex_count = 0;

  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (shouldEscape(*it, mode)) {
      if (*it == ' ' && mode == Encoding::QueryComponent) {
        space_count++;
      } else {
        hex_count++;
      }
    }
  }

  if (space_count == 0 && hex_count == 0) {
    return str;
  }

  std::string escaped;
  size_t required = str.size() + 2 * hex_count;
  escaped.reserve(required);

  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (shouldEscape(*it, mode)) {
      if (*it == ' ' && mode == Encoding::QueryComponent) {
        escaped += '+';
      } else {
        escaped += '%';
        escaped += upperhex[*it >> 4];
        escaped += upperhex[*it & 0xF];
      }
    } else {
      escaped += *it;
    }
  }

  return escaped;
}

bool ishex(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }

char unhex(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return 0;
}

// todo
std::string Encoding::unescape(const std::string& str, Encoding::Type mode) {
  size_t pct_count = 0;
  bool has_plus = false;

  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (*it == '%') {
      pct_count++;
      if (std::distance(it, str.end()) < 2 || !ishex(*(it + 1)) || !ishex(*(it + 2))) {
        throw std::runtime_error("URI unescape: invalid %");
      }

      // if we want to support IPv6, we need to allow %25 (https://tools.ietf.org/html/rfc6874#section-2)

      std::advance(it, 2);
    } else if (*it == '+') {
      has_plus = mode == QueryComponent;
    } else {
      if (mode == Host && shouldEscape(*it, mode)) {
        throw std::runtime_error("URI: host cannot escape " + std::string(1, *it));
      }
    }
  }

  if (pct_count == 0 && !has_plus) {
    return str;
  }

  std::string unescaped;
  unescaped.reserve(str.size() - 2 * pct_count);
  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (*it == '%') {
      unescaped += (unhex(*(it + 1)) << 4) + unhex(*(it + 2));
      std::advance(it, 2);
    } else if (*it == '+') {
      unescaped += ' ';
    } else {
      unescaped += *it;
    }
  }

  return unescaped;
}
