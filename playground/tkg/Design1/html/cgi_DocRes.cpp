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
  ss << "AUTH_TYPE: " << getenv("AUTH_TYPE") << std::endl;
  ss << "CONTENT_LENGTH: " << getenv("CONTENT_LENGTH") << std::endl;
  ss << "CONTENT_TYPE: " << getenv("CONTENT_TYPE") << std::endl;
  ss << "GATEWAT_INTERFACE: " << getenv("GATEWAT_INTERFACE") << std::endl;
  ss << "PATH_INFO: " << getenv("PATH_INFO") << std::endl;
  ss << "PATH_TRANSLATED: " << getenv("PATH_TRANSLATED") << std::endl;
  ss << "QUERY_STRING: " << getenv("QUERY_STRING") << std::endl;
  ss << "REMOTE_ADDR: " << getenv("REMOTE_ADDR") << std::endl;
  ss << "REMOTE_HOST: " << getenv("REMOTE_HOST") << std::endl;
  ss << "REMOET_IDENT: " << getenv("REMOET_IDENT") << std::endl;
  ss << "REMOTE_USER: " << getenv("REMOTE_USER") << std::endl;
  ss << "REQUEST_METHOD: " << getenv("REQUEST_METHOD") << std::endl;
  ss << "SCRIPT_NAME: " << getenv("SCRIPT_NAME") << std::endl;
  ss << "SERVER_NAME: " << getenv("SERVER_NAME") << std::endl;
  ss << "SERVER_PORT: " << getenv("SERVER_PORT") << std::endl;
  ss << "SERVER_PROTOCOL: " << getenv("SERVER_PROTOCOL") << std::endl;
  ss << "SERVER_SOFTWARE: " << getenv("SERVER_SOFTWARE") << std::endl;
  ss << "PROTOCOL_VAR_NAME: " << getenv("PROTOCOL_VAR_NAME") << std::endl;
  ss << "EXTENSION_VAR_NAME: " << getenv("EXTENSION_VAR_NAME") << std::endl;
  // std::cout << "Content-Length:" << ss.str().size() << std::endl;
  std::cout << std::endl;
  std::cout << ss.str();
}