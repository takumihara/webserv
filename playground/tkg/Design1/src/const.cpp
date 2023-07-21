
#include "const.hpp"

std::string alpha() { return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; }
std::string digit() { return "0123456789"; }
std::string tchar() { return "!#$%&'*+-.^_`|~" + alpha() + digit(); }
std::string vchar() { return "!#$%&'*+-.^_`|~" + alpha() + digit() + "()/:?@[]"; }
// this is not exactly true like it cannot have space at the end, but it's enough for now
std::string fieldContent() { return tchar() + " \t"; }

const std::vector<char> &CRLFVec() {
  static std::vector<char> res;
  if (res.empty()) {
    res.push_back('\r');
    res.push_back('\n');
  }
  return res;
}

const std::vector<char> &CRLFCRLFVec() {
  static std::vector<char> res;
  if (res.empty()) {
    res.push_back('\r');
    res.push_back('\n');
    res.push_back('\r');
    res.push_back('\n');
  }
  return res;
}
