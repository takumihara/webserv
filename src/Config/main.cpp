#include <iostream>

#include "Config.hpp"
#include "Parser.hpp"
#include "debug.hpp"

int main() {
  const char *file = "conf.conf";
  Parser parser;
  Config conf = parser.parse(file);
  conf.makePortServConfMap();
  DEBUG_PUTS("-----------port conf map-------------");
  conf.printPortServConfMap();
}
