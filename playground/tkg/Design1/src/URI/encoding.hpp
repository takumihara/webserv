#include <string>

namespace Encoding {
enum Type {
  Path,
  PathSegment,
  Host,
  // encodeZone,
  UserInfo,
  QueryComponent,
  Fragment
};
std::string escape(const std::string& str, Type mode);
std::string unescape(const std::string& str, Type mode);

}  // namespace Encoding
