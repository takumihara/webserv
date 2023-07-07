#include "HTML.hpp"

std::string HTML::header() {
  return "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
         "<meta charset=\"UTF-8\">\n"
         "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
         "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
         "<title>Document</title>\n</head>\n<body>";
}
std::string HTML::footer() { return "</body>\n</html>"; }

std::string HTML::aTag(const std::string &url, const std::string &text) {
  return "<p><a href=\"" + url + "\">" + text + " </a><br></p>\n";
}

std::string HTML::sanitize(const std::string &origin) {
  std::string sanitized;
  sanitized.reserve(origin.size());
  for (std::string::const_iterator c = origin.cbegin(); c != origin.cend(); c++) {
    switch (*c) {
      case '&':
        sanitized += "&amp;";
        break;
      case '<':
        sanitized += "&lt;";
        break;
      case '>':
        sanitized += "&gt;";
        break;
      case '"':
        sanitized += "&quot;";
        break;
      case '\'':
        sanitized += "&#39;";
        break;
      default:
        sanitized += *c;
    }
  }
  return sanitized;
}
