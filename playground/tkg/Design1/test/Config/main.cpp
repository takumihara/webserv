#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

std::string stringifyConfig(const char *filename);

int main(int argc, char **argv) {
  std::string path = "";
  if (argc == 2)
    path = std::string(argv[1]);
  else
    path += "./input/OK_01_basic.conf";
  std::string res = stringifyConfig(path.c_str());
  std::cout << res;
}
