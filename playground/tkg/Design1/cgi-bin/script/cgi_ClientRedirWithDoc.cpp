#include <unistd.h>

#include <iostream>
#include <sstream>

int main() {
  std::string input;
  std::cin >> input;
  std::cout << "Status:301 CRWD(CGI)" << std::endl;
  std::cout << "Location:http://example.com" << std::endl;
  std::cout << "Content-Type:text/html" << std::endl;
  std::cout << std::endl;
  std::stringstream ss;
  ss << input << std::endl;
  ss << "Client Redirection With Document CGI Response";
  // std::cout << "Content-Length:" << ss.str().size() << std::endl;
  std::cout << ss.str();
}