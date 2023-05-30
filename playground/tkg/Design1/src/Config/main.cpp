#include <iostream>

#include "Config.hpp"
#include "Parser.hpp"

int main() {
  const char *file = "conf.conf";
  Parser parser;
  Config conf = parser.parse(file);
  conf.makePortServConfMap();
  std::cout << "-----------port conf map-------------\n" << std::endl;
  conf.printPortServConfMap();
}