#include <unistd.h>

#include <iostream>
#include <sstream>

int main() {
  std::string input;
  std::cin >> input;
  std::cout << "Content-Type:text/html" << std::endl;
  // std::cout << "Status: 208" << std::endl;
  std::stringstream ss;
  ss << input << std::endl;
  ss << "CGI Response \n";
  std::cout << std::endl;
  std::cout << ss.str();
}