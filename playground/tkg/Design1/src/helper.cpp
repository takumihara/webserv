#include "helper.hpp"

#include <algorithm>

#include "Config/validation.h"
#include "const.hpp"

bool in(const std::string &str, const std::string *arr, size_t size) {
  return std::find(arr, arr + size, str) != arr + size;
}

char asciitolower(char in) {
  if (in <= 'Z' && in >= 'A') return in - ('Z' - 'z');
  return in;
}

std::string &toLower(std::string &str) {
  std::transform(str.begin(), str.end(), str.begin(), asciitolower);
  return str;
}

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

std::vector<std::string> splitToSegment(const std::string &path) {
  std::vector<std::string> segments;
  std::size_t start = 0, end = path.find('/');
  while (end != std::string::npos) {
    segments.push_back(path.substr(start, end - start));
    start = end + 1;
    end = path.find('/', start);
  }
  segments.push_back(path.substr(start));
  return segments;
}

// path must not have query
std::string getCGIExtension(const std::string &path) {
  std::vector<std::string> segments = splitToSegment(path);
  for (std::size_t i = 0; i < segments.size(); i++) {
    // remove param
    std::string trimed_seg = segments[i].substr(0, segments[i].find(';'));
    trimed_seg = trimed_seg.substr(0, trimed_seg.find(','));
    std::size_t ext_pos = trimed_seg.rfind(".cgi");
    if (ext_pos != std::string::npos && ext_pos + 4 == trimed_seg.size()) return ".cgi";
    ext_pos = trimed_seg.rfind(".php");
    if (ext_pos != std::string::npos && ext_pos + 4 == trimed_seg.size()) return ".php";
    ext_pos = trimed_seg.rfind(".py");
    if (ext_pos != std::string::npos && ext_pos + 3 == trimed_seg.size()) return ".py";
  }
  return "";
}

std::string getExtension(const std::string &path) {
  std::size_t slash_pos = path.rfind("/");
  std::string filename = path;
  if (slash_pos != std::string::npos) filename = path.substr(slash_pos + 1);
  std::size_t dot_pos = path.rfind(".");
  if (dot_pos == std::string::npos) return filename.substr(dot_pos + 1);
  return "";
}

bool isExecutable(const char *path) { return access(path, X_OK) == 0; }

bool isReadable(const char *path) { return access(path, R_OK) == 0; }

bool isWritable(const char *path) { return access(path, W_OK) == 0; }

bool isAllDirectoryWritable(std::string &path) {
  std::cout << path << std::endl;
  std::vector<std::string> segments = splitToSegment(path);
  for (std::size_t i = 1; i < segments.size(); i++) {
    std::string partial_path = "";
    for (std::size_t j = 0; j < i; j++) {
      partial_path += segments[j];
      if (j + 1 != i) partial_path += "/";
    }
    if (!isWritable(partial_path.c_str())) return false;
  }
  return true;
}