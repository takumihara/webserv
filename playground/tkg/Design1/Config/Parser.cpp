#include "Parser.hpp"

#include <cctype>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "Config.hpp"
#include "Parser.hpp"

std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("conf file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

void Parser::printTokens() {
  for (std::vector<Token>::iterator itr = tokens_.begin(); itr != tokens_.end(); itr++) {
    std::cout << itr->str_ << std::endl;
  }
}

Config Parser::parser(const char *conf_file) {
  std::string content = readFile(conf_file);
  lexer(content);
  printTokens();
  Config conf;
  return conf;
}

void Parser::lexer(std::string &input) {
  if (input.empty()) {
    throw std::runtime_error("conf file is empty");
  }
  while (input.size() != 0) {
    if (isSpace(input[0])) {
      input = skipSpaces(input);
    } else if (isReserveChar(input[0])) {
      input = addReserveToken(input);
    } else {
      input = addStringToken(input);
    }
  }
}

bool Parser::isSpace(char c) { return isspace(c); }

bool Parser::isReserveChar(char c) {
  std::string reserve(";{}");
  return reserve.find(c) != std::string::npos;
}

std::string &Parser::skipSpaces(std::string &input) {
  size_t i = 0;
  while (isspace(input[i])) i++;
  input.erase(0, i);
  return input;
}

std::string &Parser::addReserveToken(std::string &input) {
  std::string str = input.substr(0, 1);
  input.erase(0, 1);
  tokens_.push_back(Token(Token::SEMICOLON, str));
  return input;
}

std::string &Parser::addStringToken(std::string &input) {
  size_t i = 0;
  while (!isSpace(input[i]) && !isReserveChar(input[i])) i++;
  std::string str = input.substr(0, i);
  std::cout << "str: " << str << std::endl;
  tokens_.push_back(Token(Token::SEMICOLON, str));
  input.erase(0, i);
  return input;
}