#include "Parser.hpp"

#include <cctype>
#include <exception>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#include "Config.hpp"
#include "Parser.hpp"
#include "validation.h"

const std::string Parser::kReserved = ";{}";
const std::string Parser::kDefaultIP = "0.0.0.0";
const int Parser::kDefaultPort = 8080;

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
  conf_.server_confs_.push_back(ServConf(conf_));
  scope_.push(SERVER);
  return;
}

void Parser::setHost(std::string &host) {
  ServConf &serv_conf = conf_.server_confs_.back();
  if (host == "")
    serv_conf.host_.push_back(kDefaultIP);
  else
    serv_conf.host_.push_back(host);
}

void Parser::setPort(std::string &port) {
  ServConf &serv_conf = conf_.server_confs_.back();
  if (port == "")
    serv_conf.port_.push_back(kDefaultPort);
  else
    serv_conf.port_.push_back(atoi(port.c_str()));
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
    if (!validateHost(host) || !validatePort(port)) {
      // todo: invalid port or host handle
      std::cout << "invalid host or port\n";
    }
    setHost(host);
    setPort(port);
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
  if (!expectTokenType(tok, Token::STRING)) {
    // invalid grammar handle
  }
  std::string path = tok.str_;
  tok = readToken();
  if (!expectTokenType(tok, Token::OPEN_BRACE)) {
    // invalid grammar handle
  }
  ServConf &serv = conf_.server_confs_.back();
  serv.location_confs_.push_back(LocConf(path, serv));
  scope_.push(LOCATION);
  return;
}

void Parser::analyseIndex() {
  std::cout << "Analyse index\n";
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    // invalid grammar handle
  }
  if (scope_.top() == GENERAL) {
    while (expectTokenType(tok, Token::STRING)) {
      conf_.index_.push_back(tok.str_);
      tok = readToken();
    }
  } else if (scope_.top() == SERVER) {
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
  } else {
    // todo: when scope is GENERAL
  }
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammae handle (need semicolon but not)
    std::cout << "need semicolon\n";
  }
}

void Parser::analyseAutoindex() {
  std::cout << "Analyse autoindex\n";
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    // invalid grammar handle
  }
  if (scope_.top() == GENERAL) {
    conf_.autoindex_ = tok.str_ == "on";
  } else if (scope_.top() == SERVER) {
    // when scope is server
    ServConf &serv = conf_.server_confs_.back();
    serv.autoindex_ = tok.str_ == "on";

  } else if (scope_.top() == LOCATION) {
    // when scope is location
    LocConf &loc = conf_.server_confs_.back().location_confs_.back();
    loc.autoindex_ = tok.str_ == "on";
  }
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammae handle (need semicolon but not)
    std::cout << "need semicolon\n";
  }
}

void Parser::setErrorPages(std::vector<std::string> &status, std::string &path) {
  if (scope_.top() == GENERAL) {
    // when scope is server
    for (std::vector<std::string>::iterator itr = status.begin(); itr != status.end(); itr++) {
      if (conf_.error_pages_.find(*itr) != conf_.error_pages_.end()) {
        // invalid grammar handle
      }
      conf_.error_pages_[*itr] = path;
    }
  } else if (scope_.top() == SERVER) {
    // when scope is server
    ServConf &serv = conf_.server_confs_.back();
    for (std::vector<std::string>::iterator itr = status.begin(); itr != status.end(); itr++) {
      if (serv.error_pages_.find(*itr) != serv.error_pages_.end()) {
        // invalid grammar handle
      }
      serv.error_pages_[*itr] = path;
    }
  } else if (scope_.top() == LOCATION) {
    // when scope is location
    LocConf &loc = conf_.server_confs_.back().location_confs_.back();
    for (std::vector<std::string>::iterator itr = status.begin(); itr != status.end(); itr++) {
      if (loc.error_pages_.find(*itr) != loc.error_pages_.end()) {
        // invalid grammar handle
      }
      loc.error_pages_[*itr] = path;
    }
  }
}

void Parser::analyseErrorPage() {
  std::cout << "Analyse Error page\n";
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    // todo: invalid grammar handle
  }
  std::vector<std::string> status;
  while (isStatusCode(tok.str_)) {
    status.push_back(tok.str_);
    tok = readToken();
    if (!expectTokenType(tok, Token::STRING)) {
      // todo: invalid grammar handle
    }
  }
  if (!isPath(tok.str_)) {
    // todo: invalid grammar handle
  }
  setErrorPages(status, tok.str_);
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammae handle
    std::cout << "need semicolon\n";
  }
}

void Parser::analyseRedirect() {
  // todo:
  ;
}

void Parser::analyseMaxBodySize() {
  std::cout << "Analyse max body size\n";
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING) || !isAllDigit(tok.str_)) {
    // invalid grammar handle
  }
  std::cout << tok.str_ << std::endl;
  std::stringstream sstream(tok.str_);
  if (scope_.top() == GENERAL) {
    sstream >> conf_.max_body_size;
  } else if (scope_.top() == SERVER) {
    // when scope is server
    ServConf &serv = conf_.server_confs_.back();
    sstream >> serv.max_body_size;
  } else if (scope_.top() == LOCATION) {
    // when scope is location
    LocConf &loc = conf_.server_confs_.back().location_confs_.back();
    sstream >> loc.max_body_size;
  }
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammae handle (need semicolon but not)
    std::cout << "need semicolon\n";
  }
}

void Parser::analyseLimitExcept() {
  std::cout << "Analyse limit_except\n";
  if (scope_.top() != LOCATION) {
    // todo: invalid grammar
  }
  LocConf &loc = conf_.server_confs_.back().location_confs_.back();
  Token tok = readToken();
  while (expectTokenType(tok, Token::STRING) && isMethod(tok.str_)) {
    loc.allowed_methods_[tok.str_] = true;
    tok = readToken();
  }
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    // todo: invalid grammar handle
  }
  loc.allowed_methods_["HEAD"] = true;
  return;
}

Config Parser::parse(const char *conf_file) {
  std::string content = readFile(conf_file);
  lexer(content);
  while (idx_ != tokens_.size()) {
    Token cur = readToken();
    std::cout << cur.str_ << std::endl;
    if (isDirective(cur)) {
      void (Parser::*direct)() = directives_[cur.str_];
      (this->*direct)();
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
  conf_.printConfig();
  return conf_;
}

void Parser::lexer(std::string &input) {
  if (input.empty()) {
    throw std::runtime_error("conf file is empty");
  }
  while (idx_ < input.size()) {
    if (isspace(input[idx_])) {
      skipSpaces(input);
    } else if (isReservedChar(input[idx_])) {
      addReservedToken(input);
    } else {
      addStringToken(input);
    }
  }
  idx_ = 0;
}

bool Parser::isReservedChar(char c) { return Parser::kReserved.find(c) != std::string::npos; }

void Parser::skipSpaces(std::string &input) {
  while (isspace(input[idx_])) idx_++;
}

Token::t_TK_type Parser::getReserveCharType(std::string &str) {
  if (str == "{")
    return Token::OPEN_BRACE;
  else if (str == "}")
    return Token::CLOSE_BRACE;
  return Token::SEMICOLON;
}

void Parser::addToken(Token::t_TK_type type, std::string &str) { tokens_.push_back(Token(type, str)); }

void Parser::addReservedToken(std::string &input) {
  std::string str = input.substr(idx_, 1);
  idx_++;
  addToken(getReserveCharType(str), str);
}

void Parser::addStringToken(std::string &input) {
  size_t i = 0;
  while (!isspace(input[idx_ + i]) && !isReservedChar(input[idx_ + i])) i++;
  std::string str = input.substr(idx_, i);
  addToken(Token::STRING, str);
  idx_ += i;
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