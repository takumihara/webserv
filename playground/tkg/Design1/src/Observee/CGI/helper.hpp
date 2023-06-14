#pragma once

namespace CGI {
bool isMark(char c);
bool isUnreserved(char c);
bool isHex(char c);
bool isEscaped(char *escape);
bool isReserved(char c);
bool isPchar(char *c);
bool isAbsPath(char *path);
}