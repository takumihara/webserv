#ifndef CONFIG_PARSER_HPP_
#define CONFIG_PARSER_HPP_

#include <stack>
#include <string>
#include <vector>

#include "Config.hpp"

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

class Parser {
 public:
  typedef Config::ServerConf ServConf;
  typedef Config::ServerConf::LocationConf LocConf;

  Parser() : idx_(0) {
    scope_.push(GENERAL);
    directives_["limit_connection"] = &Parser::analyseLimitConnection;
    directives_["server"] = &Parser::analyseServer;
    directives_["listen"] = &Parser::analyseListen;
    directives_["server_name"] = &Parser::analyseServerName;
    directives_["root"] = &Parser::analyseRoot;
    directives_["location"] = &Parser::analyseLocation;
    directives_["index"] = &Parser::analyseIndex;
  }
  enum scope {
    GENERAL,
    SERVER,
    LOCATION,
  };
  Config parser(const char *conf_file);
  void lexer(std::string &input);
  void printTokens();
  // void printConf();
  bool isDirective(Token &tok);
  bool expectTokenType(Token &tok, Token::t_TK_type type);
  void analyseLimitConnection();
  void analyseServer();
  void analyseListen();
  void analyseServerName();
  void analyseRoot();
  void analyseLocation();
  void analyseIndex();
  void setHost(std::string &host, ServConf &conf);
  void setPort(std::string &port, ServConf &conf);

  Config conf_;
  std::vector<Token> tokens_;
  size_t idx_;
  std::map<std::string, void (Parser::*)()> directives_;
  std::stack<enum scope> scope_;
  // static std::string kReserveChars;

 private:
  bool isSpace(char c);
  bool isReserveChar(char c);
  void skipSpaces(std::string &itr);
  void addReserveToken(std::string &itr);
  void addStringToken(std::string &itr);
  void addToken(Token::t_TK_type type, std::string &str);
  Token::t_TK_type getReserveCharType(std::string &str);
  Token &readToken(void);
};

#endif