#include "Config.hpp"
#include "Parser.hpp"

int main() {
  const char *path = "conf";
  Parser parser;
  parser.parser(path);
}