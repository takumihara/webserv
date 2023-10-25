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
std::string escape(std::string& str, Type mode);
std::string unescape(std::string& str, Type mode);

}  // namespace Encoding
