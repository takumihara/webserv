#include "Parser.hpp"

#include <cctype>
#include <exception>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>

#include "Config.hpp"
#include "Parser.hpp"
#include "validation.h"

std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("conf file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

void Parser::analyseLimitConnection(void) {
  std::cout << "Analyse limit connection\n";
  if (scope_.top() != GENERAL) {
    // todo: invalid grammar
  }
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    // todo: invalid grammar handle
  }
  if (!isAllDigit(tok.str_)) {
    // todo: invalid limit connection value handling
  }
  conf_.limit_connection_ = atoi(tok.str_.c_str());
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammae handle
    std::cout << "need semicolon\n";
  }
  return;
}

void Parser::analyseServer() {
  std::cout << "Analyse server\n";
  if (scope_.top() != GENERAL) {
    // todo: invalid grammar
  }
  Token tok = readToken();
  if (expectTokenType(tok, Token::OPEN_BRACE)) {
    // todo: invalid grammar handle
  }
  conf_.server_confs_.push_back(ServConf());
  scope_.push(SERVER);
  return;
}

void Parser::setHost(std::string &host, ServConf &conf) {
  if (host == "")
    conf.host_.push_back("0.0.0.0");
  else
    conf.host_.push_back(host);
}

void Parser::setPort(std::string &port, ServConf &conf) {
  if (port == "")
    conf.port_.push_back(80);
  else
    conf.port_.push_back(atoi(port.c_str()));
}

void Parser::analyseListen() {
  std::cout << "Analyse listen\n";
  if (scope_.top() != SERVER) {
    // todo: invalid grammar
  }
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    // invalid grammar handle
  }
  while (expectTokenType(tok, Token::STRING)) {
    size_t pos = tok.str_.find(":");
    if (pos == std::string::npos) {
      // todo: invalid value handle
    }
    std::string host = tok.str_.substr(0, pos);
    std::string port = tok.str_.erase(0, pos + 1);
    std::cout << "host port " << host << " " << port << std::endl;
    ServConf &serv = conf_.server_confs_.back();
    if (!validateHost(host) || !validatePort(port)) {
      // todo: invalid port or host handle
      std::cout << "invalid host or port\n";
    }
    setHost(host, serv);
    setPort(port, serv);
    tok = readToken();
  }
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammae handle
    std::cout << "need semicolon\n";
  }
}

void Parser::analyseServerName() {
  std::cout << "Analyse server_name\n";
  if (scope_.top() != SERVER) {
    // todo: invalid grammar　handle
  }
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    // invalid grammar handle
  }
  while (expectTokenType(tok, Token::STRING)) {
    ServConf &serv = conf_.server_confs_.back();
    serv.server_names_.push_back(tok.str_);
    tok = readToken();
  }
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammae handle
    std::cout << "need semicolon\n";
  }
}

void Parser::analyseRoot() {
  std::cout << "Analyse server_root\n";
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    // todo: invalid grammar handle
  }
  if (scope_.top() == SERVER) {
    // when scope is server
    ServConf &serv = conf_.server_confs_.back();
    serv.root_ = tok.str_;
  } else if (scope_.top() == LOCATION) {
    // when scope is location
    LocConf &loc = conf_.server_confs_.back().location_confs_.back();
    loc.root_ = tok.str_;
  }
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammae handle
    std::cout << "need semicolon\n";
  }
}

void Parser::analyseLocation() {
  std::cout << "Analyse location\n";
  if (scope_.top() != SERVER) {
    // todo: invalid grammar
  }
  Token tok = readToken();
  if (expectTokenType(tok, Token::STRING)) {
    // invalid grammar handle
  }
  std::string path = tok.str_;
  tok = readToken();
  if (expectTokenType(tok, Token::OPEN_BRACE)) {
    // invalid grammar handle
  }
  conf_.server_confs_.back().location_confs_.push_back(LocConf(path));
  scope_.push(LOCATION);
  return;
}

void Parser::analyseIndex() {
  std::cout << "Analyse index\n";
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    // invalid grammar handle
  }
  if (scope_.top() == SERVER) {
    // when scope is server
    while (expectTokenType(tok, Token::STRING)) {
      ServConf &serv = conf_.server_confs_.back();
      serv.index_.push_back(tok.str_);
      tok = readToken();
    }
  } else if (scope_.top() == LOCATION) {
    // when scope is location
    while (expectTokenType(tok, Token::STRING)) {
      LocConf &loc = conf_.server_confs_.back().location_confs_.back();
      loc.index_.push_back(tok.str_);
      tok = readToken();
    }
  }
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammae handle (need semicolon but not)
    std::cout << "need semicolon\n";
  }
}

Config Parser::parser(const char *conf_file) {
  std::string content = readFile(conf_file);
  lexer(content);
  std::cout << "idx: " << idx_ << std::endl;
  while (idx_ != tokens_.size()) {
    Token cur = readToken();
    std::cout << cur.str_ << std::endl;
    if (isDirective(cur)) {
      void (Parser::*po)() = directives_[cur.str_];
      (this->*po)();
    } else if (expectTokenType(cur, Token::CLOSE_BRACE)) {
      if (scope_.size() == 0) break;
      scope_.pop();
    } else {
      // todo:: invalid grammar handle (no such directive)
      break;
    }
  }
  if (idx_ != tokens_.size() || scope_.size() != 1 || scope_.top() != GENERAL) {
    // todo: invalid grammar(unbalanced braces) handle
    std::cout << "error: unbalanced braces" << std::endl;
  }
  // printTokens();
  conf_.printConfig();
  return conf_;
}

void Parser::lexer(std::string &input) {
  if (input.empty()) {
    throw std::runtime_error("conf file is empty");
  }
  while (input.size() != 0) {
    if (isSpace(input[0])) {
      skipSpaces(input);
    } else if (isReserveChar(input[0])) {
      addReserveToken(input);
    } else {
      addStringToken(input);
    }
  }
}

bool Parser::isSpace(char c) { return isspace(c); }

bool Parser::isReserveChar(char c) {
  std::string reserve(";{}");
  return reserve.find(c) != std::string::npos;
}

void Parser::skipSpaces(std::string &input) {
  size_t i = 0;
  while (isspace(input[i])) i++;
  input.erase(0, i);
}

Token::t_TK_type Parser::getReserveCharType(std::string &str) {
  if (str == "{")
    return Token::OPEN_BRACE;
  else if (str == "}")
    return Token::CLOSE_BRACE;
  return Token::SEMICOLON;
}

void Parser::addToken(Token::t_TK_type type, std::string &str) { tokens_.push_back(Token(type, str)); }

void Parser::addReserveToken(std::string &input) {
  std::string str = input.substr(0, 1);
  input.erase(0, 1);
  addToken(getReserveCharType(str), str);
}

void Parser::addStringToken(std::string &input) {
  size_t i = 0;
  while (!isSpace(input[i]) && !isReserveChar(input[i])) i++;
  std::string str = input.substr(0, i);
  addToken(Token::STRING, str);
  input.erase(0, i);
}

Token &Parser::readToken(void) { return tokens_[idx_++]; }

bool Parser::expectTokenType(Token &tok, Token::t_TK_type type) {
  if (tok.type_ == type) return true;
  return false;
}

bool Parser::isDirective(Token &tok) {
  if (tok.type_ != Token::STRING) return false;
  if (directives_.find(tok.str_) == directives_.end()) return false;
  return true;
}

void Parser::printTokens() {
  for (std::vector<Token>::iterator itr = tokens_.begin(); itr != tokens_.end(); itr++) {
    std::cout << itr->str_ << std::endl;
  }
}