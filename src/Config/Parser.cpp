#include "Parser.hpp"

#include <cctype>
#include <exception>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#include "../debug.hpp"
#include "Config.hpp"
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
  DEBUG_PUTS("Analyse limit connection");
  if (scope_.top() != GENERAL) {
    throw std::runtime_error("limit_connection: invalid scope");
  }
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING) || !isAllDigit(tok.str_)) {
    throw std::runtime_error("limit_connection: invalid type or Not AllDigit");
  }
  conf_.limit_connection_ = atoi(tok.str_.c_str());
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("limit_connection: invalid grammer. need semicolon");
  }
  return;
}

void Parser::setDefaultServer() {
  ServerConf serv = ServerConf(conf_.common_);
  serv.server_names_.push_back("_");
  conf_.server_confs_.push_back(serv);
}

void Parser::analyseServer() {
  DEBUG_PUTS("Analyse server");
  if (scope_.top() != GENERAL) {
    throw std::runtime_error("server: invalid scope");
  }
  Token tok = readToken();
  if (!expectTokenType(tok, Token::OPEN_BRACE)) {
    throw std::runtime_error("server: invalid grammar, need Open brace");
  }
  conf_.server_confs_.push_back(ServerConf(conf_.common_));
  scope_.push(SERVER);
  return;
}

void Parser::setHost(std::string &host) {
  ServerConf &serv_conf = conf_.server_confs_.back();
  if (host == "")
    serv_conf.host_ = kDefaultIP;
  else
    serv_conf.host_ = host;
}

void Parser::setPort(std::string &port) {
  ServerConf &serv_conf = conf_.server_confs_.back();
  if (port == "")
    serv_conf.port_ = kDefaultPort;
  else
    serv_conf.port_ = atoi(port.c_str());
}

void Parser::analyseListen() {
  DEBUG_PUTS("Analyse listen");
  if (scope_.top() != SERVER) {
    throw std::runtime_error("listen: invalid scope");
  }
  if (!conf_.server_confs_.back().host_.empty()) {
    throw std::runtime_error("listen: duplicate listen");
  }
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    throw std::runtime_error("listen: invalid type");
  }
  size_t pos = tok.str_.find(":");
  if (pos == std::string::npos) {
    throw std::runtime_error("listen: invalid grammar, need colon");
  }
  std::string host = tok.str_.substr(0, pos);
  std::string port = tok.str_.erase(0, pos + 1);
  if (!validateHost(host) || !validatePort(port)) {
    throw std::runtime_error("listen: invalid host ot port");
  }
  setHost(host);
  setPort(port);
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("listen: invalid grammar, need semicolon");
  }
}

void Parser::analyseServerName() {
  DEBUG_PUTS("Analyse server_name");
  if (scope_.top() != SERVER) {
    throw std::runtime_error("server_name: invalid scope");
  }
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    throw std::runtime_error("server_name: invalid type");
  }
  while (expectTokenType(tok, Token::STRING)) {
    ServerConf &serv = conf_.server_confs_.back();
    serv.server_names_.push_back(tok.str_);
    tok = readToken();
  }
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("server_name: invalid grammar, need semicolon");
  }
}

void Parser::analyseRoot() {
  DEBUG_PUTS("Analyse server_root");
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    throw std::runtime_error("server_name: invalid type");
  }
  if (scope_.top() == SERVER) {
    // when scope is server
    ServerConf &serv = conf_.server_confs_.back();
    serv.common_.root_ = tok.str_;
  } else if (scope_.top() == LOCATION) {
    // when scope is location
    LocationConf &loc = conf_.server_confs_.back().location_confs_.back();
    loc.common_.root_ = tok.str_;
  }
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("root: invalid grammar, need semicolon");
  }
}

void Parser::setDefaultLocation() {
  ServerConf &serv = conf_.server_confs_.back();
  LocationConf loc = LocationConf("/", serv.redirect_, serv.common_);
  conf_.server_confs_.push_back(serv);
}

void Parser::analyseLocation() {
  DEBUG_PUTS("Analyse location");
  if (scope_.top() != SERVER) {
    throw std::runtime_error("location: invalid scope");
  }
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    throw std::runtime_error("location: invalid type");
  }
  std::string path = tok.str_;
  tok = readToken();
  if (!expectTokenType(tok, Token::OPEN_BRACE)) {
    throw std::runtime_error("location: invalid grammar, need Open brace");
  }
  ServerConf &serv = conf_.server_confs_.back();
  serv.location_confs_.push_back(LocationConf(path, serv.redirect_, serv.common_));
  scope_.push(LOCATION);
  return;
}

void Parser::analyseIndex() {
  DEBUG_PUTS("Analyse index");
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    throw std::runtime_error("index: invalid type");
  }
  if (scope_.top() == GENERAL) {
    while (expectTokenType(tok, Token::STRING)) {
      if (std::find(conf_.common_.index_.begin(), conf_.common_.index_.end(), tok.str_) == conf_.common_.index_.end())
        conf_.common_.index_.push_back(tok.str_);
      tok = readToken();
    }
  } else if (scope_.top() == SERVER) {
    while (expectTokenType(tok, Token::STRING)) {
      ServerConf &serv = conf_.server_confs_.back();
      if (std::find(serv.common_.index_.begin(), serv.common_.index_.end(), tok.str_) == serv.common_.index_.end())
        serv.common_.index_.push_back(tok.str_);
      tok = readToken();
    }
  } else if (scope_.top() == LOCATION) {
    while (expectTokenType(tok, Token::STRING)) {
      LocationConf &loc = conf_.server_confs_.back().location_confs_.back();
      if (std::find(loc.common_.index_.begin(), loc.common_.index_.end(), tok.str_) == loc.common_.index_.end())
        loc.common_.index_.push_back(tok.str_);
      tok = readToken();
    }
  } else {
    throw std::runtime_error("index: invalid scope");
  }
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("index: invalid grammar, need semicolon");
  }
}

template <class T>
void setAutoindex(T &conf, const std::string &flag) {
  if (flag == "on")
    conf.common_.autoindex_ = true;
  else if (flag == "off")
    conf.common_.autoindex_ = false;
  else {
    throw std::runtime_error("autoindex: invalid value");
  }
}

void Parser::analyseAutoindex() {
  DEBUG_PUTS("Analyse autoindex");
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    throw std::runtime_error("autoindex: invalid type");
  }
  if (scope_.top() == GENERAL) {
    setAutoindex(conf_, tok.str_);
  } else if (scope_.top() == SERVER) {
    ServerConf &serv = conf_.server_confs_.back();
    setAutoindex(serv, tok.str_);

  } else if (scope_.top() == LOCATION) {
    LocationConf &loc = conf_.server_confs_.back().location_confs_.back();
    setAutoindex(loc, tok.str_);
  }
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("index: invalid grammar, need semicolon");
  }
}

template <class T>
void setErrorPages(T &conf, std::vector<std::string> &status, std::string &path) {
  for (std::vector<std::string>::iterator itr = status.begin(); itr != status.end(); itr++) {
    if (conf.common_.error_pages_.find(*itr) != conf.common_.error_pages_.end()) {
      throw std::runtime_error("error_page: duplicate erro_page path");
    }
    conf.common_.error_pages_[*itr] = path;
  }
}

void Parser::analyseErrorPage() {
  DEBUG_PUTS("Analyse Error page");
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING)) {
    throw std::runtime_error("error_page: invalid type");
  }
  std::vector<std::string> status;
  while (isStatusCode(tok.str_)) {
    status.push_back(tok.str_);
    tok = readToken();
    if (!expectTokenType(tok, Token::STRING)) {
      throw std::runtime_error("error_page: invalid type");
    }
  }
  if (!isPath(tok.str_)) {
    throw std::runtime_error("error_page: invalid path");
  }
  if (scope_.top() == GENERAL) {
    setErrorPages(conf_, status, tok.str_);
  } else if (scope_.top() == SERVER) {
    ServerConf &serv = conf_.server_confs_.back();
    setErrorPages(serv, status, tok.str_);
  } else if (scope_.top() == LOCATION) {
    LocationConf &loc = conf_.server_confs_.back().location_confs_.back();
    setErrorPages(loc, status, tok.str_);
  }
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("error_page: invalid grammar, need semicolon");
  }
}

void Parser::setRedirect(std::string &status, std::string &uri, scope scp) {
  if (scp == SERVER) {
    ServerConf &serv = conf_.server_confs_.back();
    serv.redirect_ = std::pair<std::string, std::string>(status, uri);
  } else if (scp == LOCATION) {
    LocationConf &loc = conf_.server_confs_.back().location_confs_.back();
    loc.redirect_ = std::pair<std::string, std::string>(status, uri);
  } else {
    throw std::runtime_error("redirect: invalid scope");
  }
}

void Parser::analyseRedirect() {
  DEBUG_PUTS("Analyse redirect");
  Token tok = readToken();
  while (expectTokenType(tok, Token::STRING) && is3xxStatus(tok.str_)) {
    std::string status = tok.str_;
    tok = readToken();
    if (!expectTokenType(tok, Token::STRING) || !isURL(tok.str_)) {
      throw std::runtime_error("error_page: invalid type or URL");
    }
    setRedirect(status, tok.str_, scope_.top());
    tok = readToken();
  }
}

void Parser::analyseMaxBodySize() {
  DEBUG_PUTS("Analyse max body size");
  Token tok = readToken();
  if (!expectTokenType(tok, Token::STRING) || !isAllDigit(tok.str_)) {
    throw std::runtime_error("Max_body_size: invalid type or Not AllDigit");
  }
  std::stringstream sstream(tok.str_);
  if (scope_.top() == GENERAL) {
    sstream >> conf_.common_.max_body_size_;
  } else if (scope_.top() == SERVER) {
    ServerConf &serv = conf_.server_confs_.back();
    sstream >> serv.common_.max_body_size_;
  } else if (scope_.top() == LOCATION) {
    LocationConf &loc = conf_.server_confs_.back().location_confs_.back();
    sstream >> loc.common_.max_body_size_;
  }
  tok = readToken();
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("error_page: invalid grammar, need semicolon");
  }
}

void Parser::analyseLimitExcept() {
  DEBUG_PUTS("Analyse limit_except");
  if (scope_.top() != LOCATION) {
    throw std::runtime_error("error_page: invalid scope");
  }
  LocationConf &loc = conf_.server_confs_.back().location_confs_.back();
  Token tok = readToken();
  while (expectTokenType(tok, Token::STRING) && isMethod(tok.str_)) {
    loc.allowed_methods_[tok.str_] = true;
    tok = readToken();
  }
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("error_page: invalid grammar, need semicolon");
  }
  if (loc.allowed_methods_["GET"] == true) {
    loc.allowed_methods_["HEAD"] = true;
  }
  return;
}

void Parser::analyseCGIExtension() {
  DEBUG_PUTS("Analyse CGI Extension");
  if (scope_.top() != LOCATION) {
    throw std::runtime_error("cgi_extension: invalid scope");
  }
  LocationConf &loc = conf_.server_confs_.back().location_confs_.back();
  Token tok = readToken();
  while (expectTokenType(tok, Token::STRING) && isCGIExtension(tok.str_)) {
    loc.cgi_exts_.push_back(tok.str_);
    tok = readToken();
  }
  if (!expectTokenType(tok, Token::SEMICOLON)) {
    throw std::runtime_error("cgi_extension: invalid grammar, need semicolon");
  }
}

Config Parser::parse(const char *conf_file) {
  std::string content = readFile(conf_file);
  lexer(content);
  while (idx_ != tokens_.size()) {
    Token cur = readToken();
    if (isDirective(cur)) {
      void (Parser::*direct)() = directives_[cur.str_];
      (this->*direct)();
    } else if (expectTokenType(cur, Token::CLOSE_BRACE)) {
      if (scope_.size() == 0) break;
      if (scope_.top() == GENERAL && conf_.server_confs_.size() == 0)
        setDefaultServer();
      else if (scope_.top() == SERVER && conf_.server_confs_.back().location_confs_.size() == 0)
        setDefaultLocation();
      scope_.pop();
    } else {
      throw std::runtime_error("parse: no such directive");
      break;
    }
  }
  if (idx_ != tokens_.size() || scope_.size() != 1 || scope_.top() != GENERAL) {
    throw std::runtime_error("error_page: unbalanced braces");
  }
#ifdef DEBUG
  conf_.printConfig();
#endif
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
