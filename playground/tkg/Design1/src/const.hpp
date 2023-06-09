#pragma once

#include <string>

#define SOCKET_READ_SIZE 100
#define FILE_READ_SIZE 10

#define SOCKET_WRITE_SIZE 1000

#define DEFAULT_HTTP_PORT 80
#define DEFAULT_HTTPS_PORT 443

#define SP ' '
#define CRLF "\r\n"
#define OWS " \t"

std::string alpha();
std::string digit();
std::string tchar();
std::string vchar();
std::string fieldContent();
