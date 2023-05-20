#include <iostream>

#include "Config.hpp"
#include "Parser.hpp"

int main() {
  const char *path = "conf.conf";
  Parser parser;
  Config conf = parser.parser(path);
  conf.makePortServConfMap();
  std::cout << "-----------port conf map-------------\n";
  conf.printPortServConfMap();
}