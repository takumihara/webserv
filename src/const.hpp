#pragma once

#include <map>
#include <string>
#include <vector>

#define SOCKET_READ_SIZE 1024
#define FILE_READ_SIZE 1024
#define FILE_WRITE_SIZE 1024

#define SOCKET_WRITE_SIZE 1024

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
const std::vector<char> &CRLFVec();
const std::vector<char> &CRLFCRLFVec();
