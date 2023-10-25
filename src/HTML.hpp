#pragma once
#include <string>

class HTML {
 public:
  HTML() {}
  ~HTML() {}
  static std::string header();
  static std::string footer();
  static std::string aTag(const std::string &url, const std::string &text);
  static std::string sanitize(std::string &origin);
  static std::string getDefaultErrorPage(const std::string &status, const std::string &reason);
};
