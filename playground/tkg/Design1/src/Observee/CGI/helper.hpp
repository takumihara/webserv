#pragma once
#include <string>
#include <vector>

namespace CGIValidation {
bool isMark(const char c);
bool isUnreserved(const char c);
bool isHex(const char c);
bool isEscaped(const char *escape);
bool isReserved(const char c);
bool isPchar(const char *c);
bool isAbsPath(const char *path);
bool isAbsURI(const std::string &raw_uri);
std::vector<std::string> ExtractLines(const std::string &data);
}  // namespace CGIValidation