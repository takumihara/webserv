#ifndef CONFIG_PARSER_HPP_
#define CONFIG_PARSER_HPP_

#include <string>
#include <vector>

#include "Config.hpp"

class Token {
 public:
  enum type {
    SEMICOLON,
    OPEN_BRACE,
    CLOSE_BRACE,
    STRING,
  };
  Token(enum type type, std::string str) : type_(type), str_(str){};
  enum type type_;
  std::string str_;
};

class Parser {
 public:
  // typedef std::string::iterator std::string;

  Config parser(const char *conf_file);
  void lexer(std::string &input);
  void printTokens();

  std::vector<Token> tokens_;
  // static std::string kReserveChars;

 private:
  bool isSpace(char c);
  bool isReserveChar(char c);
  void skipSpaces(std::string &itr);
  void addReserveToken(std::string &itr);
  void addStringToken(std::string &itr);
};

#endif