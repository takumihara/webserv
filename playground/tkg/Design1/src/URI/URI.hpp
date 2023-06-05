#pragma once

#include <string>

#include "UserInfo.hpp"

enum Encoding {
  encodePath,
  encodePathSegment,
  encodeHost,
  encodeZone,
  encodeUserPassword,
  encodeQueryComponent,
  encodeFragment
};

class URI {
 public:
  // todo: want it private
  URI(){};
  static URI* parse(const std::string& uri);
  static URI* parseRequestURI(const std::string& uri);

  const std::string& getScheme() const;
  const std::string& getHost() const;
  const std::string& getPath() const;
  const std::string& getQuery() const;
  const std::string& getFragment() const;
  const UserInfo* getUserInfo() const;
  const std::string& getOpaque() const;
  bool isForceQuery() const;
  bool isOmitHost() const;

  const URI& resolveReference(const URI& ref) const;

 private:
  URI(std::string raw_uri, bool via_request);

  std::string scheme_;
  std::string host_;     // (authority) host
  UserInfo* user_info_;  // (authority) username and password information
  std::string path_;     // path (relative paths may omit leading slash)
  std::string raw_path_;
  std::string raw_query_;     // encoded query values, without '?'
  std::string fragment_;      // fragment for references, without '#'
  std::string raw_fragment_;  // encoded fragment hint (see EscapedFragment method)

  std::string opaque_;  // encoded opaque data

  // omit_host_ distinguishes myscheme:/path	vs myscheme:///path
  bool omit_host_;  // do not emit empty host (authority)

  // force_query distinguishes / and /?
  // this helps to check defined(R.query)
  // if (R.path == "") then
  //    T.path = Base.path;
  //    if defined(R.query) then
  //       T.query = R.query;
  //    else
  //       T.query = Base.query;
  //    endif;
  // else ...
  // (from RFC 3986)
  bool force_query_;

  void setFragment(const std::string& fragment);
  void setPath(const std::string& path);
};
