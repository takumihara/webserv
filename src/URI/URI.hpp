#pragma once

#include <map>
#include <string>
#include <vector>

#include "UserInfo.hpp"

class URI {
 public:
  ~URI() { delete user_info_; }
  static URI* parse(const std::string& uri);
  static URI* parseRequestURI(const std::string& uri);

  const std::string& getScheme() const;
  const std::string& getHost() const;
  const std::string& getPort() const;
  const std::string& getPath() const;
  const std::string& getRawQuery() const;
  const std::map<std::string, std::vector<std::string> >& getQuery() const;
  const std::string& getFragment() const;
  const UserInfo* getUserInfo() const;
  std::string getUsername() const;
  std::string getPassword() const;
  const std::string& getOpaque() const;
  std::string recompose();
  bool isForceQuery() const;
  bool isOmitHost() const;

  std::string escapedPath();

  URI* resolveReference(URI& ref);

 private:
  URI() : user_info_(NULL), omit_host_(false), force_query_(false){};
  URI(std::string raw_uri, bool via_request);

  std::string scheme_;
  std::string host_;     // (authority) host
  std::string port_;     // (authority) port
  UserInfo* user_info_;  // (authority) username and password information
  std::string path_;     // path (relative paths may omit leading slash)
  std::string raw_path_;
  std::map<std::string, std::vector<std::string> > query_;
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

  void setFragment(std::string& fragment);
  void setPath(std::string& path);
  void parseAndSetQuery(std::string raw_query);
  void parseAndSetAuthority(std::string authority);
};
