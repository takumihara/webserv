#include <unistd.h>

#include <iostream>
#include <sstream>

int main() {
  std::string input;
  std::cin >> input;
  std::cout << "Content-Type:text/html" << std::endl;
  std::cout << std::endl;

  std::stringstream ss;
  char cwd[1028];
  getcwd(cwd, 1028);
  ss << input << std::endl;
  ss << cwd << std::endl;
  std::cout << ss.str();
}