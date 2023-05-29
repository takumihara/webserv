#ifndef CONFIG_TOKEN_HPP_
#define CONFIG_TOKEN_HPP_

#include <string>

class Token {
 public:
  typedef enum type {
    SEMICOLON,
    OPEN_BRACE,
    CLOSE_BRACE,
    STRING,
  } t_TK_type;
  Token(enum type type, std::string str) : type_(type), str_(str){};
  enum type type_;
  std::string str_;
};

#endif