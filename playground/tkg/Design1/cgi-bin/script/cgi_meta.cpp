#include <unistd.h>

#include <iostream>

int main() {
  extern char **environ;
  std::string input;
  std::cin >> input;
  std::cout << "Status:200 " << std::endl;
  std::cout << "Content-Type:text/plain" << std::endl;
  std::cout << std::endl;
  std::cout << input << std::endl;
  std::cout << "AUTH_TYPE: " << getenv("AUTH_TYPE") << std::endl;
  std::cout << "CONTENT_LENGTH: " << getenv("CONTENT_LENGTH") << std::endl;
  std::cout << "CONTENT_TYPE: " << getenv("CONTENT_TYPE") << std::endl;
  std::cout << "GATEWAT_INTERFACE: " << getenv("GATEWAT_INTERFACE") << std::endl;
  std::cout << "PATH_INFO: " << getenv("PATH_INFO") << std::endl;
  std::cout << "PATH_TRANSLATED: " << getenv("PATH_TRANSLATED") << std::endl;
  std::cout << "QUERY_STRING: " << getenv("QUERY_STRING") << std::endl;
  std::cout << "REMOTE_ADDR: " << getenv("REMOTE_ADDR") << std::endl;
  std::cout << "REMOTE_HOST: " << getenv("REMOTE_HOST") << std::endl;
  std::cout << "REMOET_IDENT: " << getenv("REMOET_IDENT") << std::endl;
  std::cout << "REMOTE_USER: " << getenv("REMOTE_USER") << std::endl;
  std::cout << "REQUEST_METHOD: " << getenv("REQUEST_METHOD") << std::endl;
  std::cout << "SCRIPT_NAME: " << getenv("SCRIPT_NAME") << std::endl;
  std::cout << "SERVER_NAME: " << getenv("SERVER_NAME") << std::endl;
  std::cout << "SERVER_PORT: " << getenv("SERVER_PORT") << std::endl;
  std::cout << "SERVER_PROTOCOL: " << getenv("SERVER_PROTOCOL") << std::endl;
  std::cout << "SERVER_SOFTWARE: " << getenv("SERVER_SOFTWARE") << std::endl;
  std::cout << "PROTOCOL_VAR_NAME: " << getenv("PROTOCOL_VAR_NAME") << std::endl;
  std::cout << "EXTENSION_VAR_NAME: " << getenv("EXTENSION_VAR_NAME") << std::endl;
}