#include "HTML.hpp"

#include <sstream>

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

std::string HTML::getDefaultErrorPage(const std::string &status, const std::string &reason) {
  std::stringstream ss;
  ss << "<html>\n<head>\n<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n";
  ss << "<title> Error" << status << " " << reason << "</title>\n";
  ss << "<style>\n<!--\n	body {font-family: arial,sans-serif}\n	img { border:none; "
        "}\n//-->\n</style>\n</head>\n<body>\n<blockquote>\n";
  ss << "<h2>Error " << status << " " << reason << "</h2>\n";
  ss << "<hr noshade>\n";
  ss << "<p>Our apologies for the temporary inconvenience. The requested URL was not found on this server.\n</p>"
        "</blockquote>\n";
  ss << "<p></p><h2 style=\"text-align:center\">Webserv</h2></body>\n</html>";
  return ss.str();
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
