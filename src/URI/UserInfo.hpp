#pragma once

#include <string>

class UserInfo {
 public:
  UserInfo(){};
  UserInfo(const std::string& username) : username(username){};
  UserInfo(const std::string& username, const std::string& password) : username(username), password(password){};
  const std::string& getUsername() const;
  const std::string& getPassword() const;
  std::string getString() const;

 private:
  std::string username;
  std::string password;
};
