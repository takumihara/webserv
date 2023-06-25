#include <string>

bool in(const std::string &str, const std::string *arr, size_t size);
std::string &toLower(std::string &str);
std::string escape(const std::string &str);
std::string trimOws(const std::string &str);
std::string trimUntilCRLF(const std::string &str);
std::string getCGIExtension(const std::string &path);
std::string getExtension(const std::string &path);
bool isExecutable(const char *path);
bool isReadable(const char *path);
bool isWritable(const char *path);
bool isAllDirectoryWritable(std::string &path);