#include <iostream>

#include "Config.hpp"
#include "Parser.hpp"

int main() {
  const char *file = "conf.conf";
  Parser parser;
  Config conf = parser.parser(file);
  // todo: check servername duplication
  conf.makePortServConfMap();
  std::cout << "-----------port conf map-------------\n";
  conf.printPortServConfMap();
  std::string name = "/test22";
  Config::ServConf *serv = conf.getServConfig(8080, name);
  std::string path = "/hello";
  Config::LocConf loc = conf.getLocationConfig(serv, path);
  std::cout << "getLoc path:" << loc.path_ << " root: " << loc.root_ << std::endl;
}