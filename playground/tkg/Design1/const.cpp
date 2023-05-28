
#include "const.hpp"

std::string alpha() { return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; }
std::string digit() { return "0123456789"; }
std::string tchar() { return "!#$%&'*+-.^_`|~" + alpha() + digit(); }
std::string vchar() { return "!#$%&'*+-.^_`|~" + alpha() + digit() + "()/:?@[]"; }
