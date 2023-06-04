#include "UserInfo.hpp"

const std::string& UserInfo::getUsername() const { return username; }
const std::string& UserInfo::getPassword() const { return password; }
std::string UserInfo::getString() const { return username + ":" + password; }
