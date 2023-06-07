#include "URI.hpp"

#include <algorithm>
#include <utility>

#include "../helper.hpp"
#include "encoding.hpp"
#include "helper.hpp"
#include "validation.h"

using Encoding::Fragment;
using Encoding::Path;

std::pair<std::string, UserInfo*> parseAuthority(std::string authority);
bool validUserinfo(const std::string& user_info);
std::string parseHost(const std::string& host);
bool validRegNameHost(const std::string& host);
bool validOptionalPort(std::string port);
std::string parseScheme(std::string& raw_url);
bool stringContainsCTLByte(const std::string& str);

URI* URI::parse(const std::string& raw_uri) {
  const size_t fragment_pos = raw_uri.find("#");
  std::string fragment;
  if (fragment_pos != std::string::npos) {
    fragment = raw_uri.substr(fragment_pos + 1);
  }

  URI* uri = new URI(raw_uri.substr(0, fragment_pos), false);
  uri->setFragment(fragment);

  return uri;
}

URI* URI::parseRequestURI(const std::string& uri) { return new URI(uri, true); }

URI::URI(std::string raw_uri, bool via_request) : user_info_(NULL), omit_host_(false), force_query_(false) {
  // todo:
  if (stringContainsCTLByte(raw_uri)) {
    throw std::runtime_error("URI: invalid control character in URL");
  }

  // path-empty
  if (raw_uri.empty() && via_request) {
    throw std::runtime_error("URI: invalid URI for request");
  }

  if (raw_uri == "*") {
    path_ = "*";
    return;
  }

  scheme_ = parseScheme(raw_uri);
  toLower(scheme_);

  if (*raw_uri.rbegin() == '?' && std::count(raw_uri.begin(), raw_uri.end(), '?') == 1) {
    raw_uri.pop_back();
    force_query_ = true;
  } else {
    size_t query_pos = raw_uri.find("?");
    if (query_pos != std::string::npos) {
      raw_query_ = raw_uri.substr(query_pos + 1);
      raw_uri = raw_uri.substr(0, query_pos);
    }
  }

  if (*raw_uri.begin() != '/') {
    if (scheme_ != "") {
      // path-rootless (RFC3986)
      opaque_ = raw_uri;
      return;
    }
    if (via_request) {
      throw std::runtime_error("URI: invalid URI for request");
    }

    // its the same logic as you can use ":" as delim for scheme
    std::string segment = raw_uri.substr(0, raw_uri.find("/"));
    if (segment.find(":") != std::string::npos) {
      throw std::runtime_error("first path segment in URL cannot contain semi-colon");
    }
  }

  bool parse_authority = false;
  if (scheme_ != "") {
    // URI
    if (raw_uri.find("//") == 0) {
      // authority path-abempty
      parse_authority = true;
    } else if (*raw_uri.begin() == '/') {
      omit_host_ = true;
    }
  } else if (!via_request && raw_uri.find("///") != 0 && raw_uri.find("//") == 0) {
    // relative-part & "//" authority path-abempty
    parse_authority = true;
  }

  if (parse_authority) {
    std::string authority = raw_uri.substr(2);
    size_t authority_end = authority.find("/");
    if (authority_end != std::string::npos) {
      raw_uri = authority.substr(authority_end);
      authority = authority.substr(0, authority_end);
    } else {
      raw_uri = "";
    }
    std::pair<std::string, UserInfo*> res = parseAuthority(authority);
    host_ = res.first;
    user_info_ = res.second;
  }

  setPath(raw_uri);
}

// todo(thara): refactor
std::pair<std::string, UserInfo*> parseAuthority(std::string authority) {
  std::string host;
  size_t at_pos = authority.find("@");
  if (at_pos == std::string::npos) {
    host = parseHost(authority);
    return std::make_pair(host, (UserInfo*)NULL);
  }
  host = parseHost(authority.substr(at_pos + 1));

  std::string raw_user_info = authority.substr(0, at_pos);
  if (!validUserinfo(raw_user_info)) {
    throw std::runtime_error("URI: invalid userinfo");
  }

  size_t colon_pos = raw_user_info.find(":");
  UserInfo* user;
  if (colon_pos == std::string::npos) {
    raw_user_info = Encoding::unescape(raw_user_info, Encoding::UserInfo);
    user = new UserInfo(raw_user_info);
  } else {
    std::string username = raw_user_info.substr(0, colon_pos);
    std::string password = raw_user_info.substr(colon_pos + 1);
    username = Encoding::unescape(username, Encoding::UserInfo);
    password = Encoding::unescape(password, Encoding::UserInfo);
    user = new UserInfo(username, password);
  }
  return std::make_pair(host, user);
}

// todo(thara): unescape in the caller
// validUserinfo reports whether s is a valid userinfo string per RFC 3986
// userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
// unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
// sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
//                / "*" / "+" / "," / ";" / "="
// It doesn't validate pct-encoded. The caller does that via func unescape.
bool validUserinfo(const std::string& user_info) {
  for (std::string::const_iterator it = user_info.begin(); it != user_info.end(); ++it) {
    if (isUnreserved(*it) || isSubDelims(*it) || *it == ':') {
      continue;
    } else {
      return false;
    }
  }
  return std::count(user_info.begin(), user_info.end(), ':') <= 1;
}

std::string parseHost(const std::string& host) {
  if (*host.begin() == '[') {
    // IP-literal
    size_t end = host.find("]");
    if (end == std::string::npos) {
      throw std::runtime_error("URI: missing ']' in host");
    }
    throw std::runtime_error("URI: IP-literal is not supported");
  }
  size_t colon_pos = host.find(":");
  if (!validRegNameHost(host.substr(0, colon_pos))) {
    throw std::runtime_error("URI: invalid host");
  }
  if (colon_pos != std::string::npos) {
    std::string port = host.substr(colon_pos);
    if (!validOptionalPort(port)) {
      throw std::runtime_error("URI: invalid optional port");
    }
  }
  return Encoding::unescape(host, Encoding::Host);
}

// validRegNameHost does not check pct-encoded. The caller does that via func unescape.
bool validRegNameHost(const std::string& host) {
  for (std::string::const_iterator it = host.begin(); it != host.end(); ++it) {
    if (isUnreserved(*it) || isSubDelims(*it)) {
      continue;
    } else {
      return false;
    }
  }
  return true;
}

bool validOptionalPort(std::string port) {
  if (port == "") {
    return true;
  }
  if (port[0] != ':') {
    return false;
  }
  return isAllDigit(port.substr(1));
}

// parseScheme puts the rest of the raw_url into the raw_url parameter and returns the scheme
std::string parseScheme(std::string& raw_url) {
  for (std::string::const_iterator it = raw_url.begin(); it != raw_url.end(); ++it) {
    if (std::isalpha(*it)) {
      continue;
    } else if (std::isdigit(*it) || *it == '+' || *it == '-' || *it == '.') {
      if (it == raw_url.begin()) {
        return "";
      }
      // URI always has scheme and any relative-part should have "/" before ":" therefore we can treat first ":" as
      // delimiter between scheme and host
    } else if (*it == ':') {
      if (it == raw_url.begin()) {
        throw std::runtime_error("missing protocol scheme");
      }
      std::string scheme = raw_url.substr(0, it - raw_url.begin());
      raw_url.erase(raw_url.begin(), it + 1);
      return scheme;
    } else {
      return "";
    }
  }
  return "";
}

void URI::setFragment(const std::string& fragment) {
  fragment_ = Encoding::unescape(fragment, Encoding::Fragment);
  std::string escaped = Encoding::escape(fragment, Encoding::Fragment);
  if (escaped == fragment) {
    raw_fragment_ = "";
  } else {
    raw_fragment_ = fragment;
  }
}

void URI::setPath(const std::string& path) {
  path_ = unescape(path, Encoding::Path);
  if (path == escape(path, Encoding::Path)) {
    raw_path_ = "";
  } else {
    raw_path_ = path;
  }
}

bool stringContainsCTLByte(const std::string& str) {
  for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
    if (*it < 32 || *it == 127) {
      return true;
    }
  }
  return false;
}

// merge merges the base URI with the ref URI and returns the merged URI
// if ref is an absolute URI, then ref is returned
std::string merge(const std::string& base, const std::string& ref) {
  if (ref == "") {
    return base;
  }
  if (ref[0] != '/') {
    size_t last_slash = base.rfind("/");
    if (last_slash == std::string::npos) {
      return ref;
    } else {
      return base.substr(0, last_slash + 1) + ref;
    }
  }
  return ref;
}

std::string removeDotSegments(const std::string& path) {
  if (path == "") {
    return "";
  }

  std::string elem;
  // this a is space for `/`
  std::string dst = "a";
  std::string remaining;
  bool found = true;
  bool first = true;

  if (path[0] == '/') {
    remaining = path.substr(1);
  } else {
    remaining = path;
  }

  while (found) {
    std::cout << dst << std::endl;
    size_t first_slash = remaining.find("/");
    elem = remaining.substr(0, first_slash);
    if (first_slash == std::string::npos) {
      found = false;
    } else {
      remaining = remaining.substr(first_slash + 1);
    }

    if (elem == ".") {
      continue;
    } else if (elem == "..") {
      size_t last_slash = dst.rfind("/");
      if (last_slash == std::string::npos) {
        dst = "a";
        first = true;
      } else {
        dst = dst.substr(0, last_slash);
        first = false;
      }
    } else {
      if (!first) {
        dst += "/";
      }
      dst += elem;
      first = false;
    }
  }

  if ((elem == "." || elem == "..") && dst.size() != 1) {
    dst += "/";
  }
  dst[0] = '/';
  return dst;
}

// caller has to be absolute-URI
URI* URI::resolveReference(const URI& ref) const {
  URI* uri = new URI(ref);

  if (ref.scheme_ == "") {
    uri->scheme_ = scheme_;
  }
  if (ref.scheme_ != "" || ref.host_ != "" || ref.user_info_ != NULL) {
    // if it has authority
    uri->setPath(removeDotSegments(ref.escapedPath()));
    return uri;
  }
  uri->host_ = host_;
  uri->user_info_ = user_info_;
  std::string path = merge(escapedPath(), ref.escapedPath());
  path = removeDotSegments(path);
  uri->setPath(path);
  if (ref.path_ == "" && !ref.force_query_ && ref.raw_query_ == "") {
    uri->raw_query_ = raw_query_;
  }
  return uri;
}

std::string URI::recompose() const {
  std::string res;
  if (scheme_ != "") {
    res += scheme_ + ":";
  }
  if (opaque_ != "") {
    res += opaque_;
  } else {
    if (host_ != "" || path_ != "" || user_info_ != NULL) {
      res += "//";
    }
    if (user_info_ != NULL) {
      res += user_info_->getString() + "@";
    }
    res += Encoding::escape(host_, Encoding::Host);

    std::string path = escapedPath();
    if (path != "" && path[0] != '/' && host_ != "") {
      path = "/";
    }
    if (res.size() == 0) {
      // RFC 3986 4.2
      std::string segment = path.substr(0, path.find("/"));
      if (segment.find(":") != std::string::npos) {
        res += "./";
      }
    }
    res += path;
  }
  if (force_query_ || raw_query_ != "") {
    res += "?" + raw_query_;
  }
  if (fragment_ != "") {
    res += "#" + escape(fragment_, Encoding::Fragment);
  }
  return res;
}

const std::string& URI::getScheme() const { return scheme_; };
const std::string& URI::getHost() const { return host_; };
const std::string& URI::getPath() const { return path_; };
const std::string& URI::getQuery() const { return raw_query_; }
const std::string& URI::getFragment() const { return fragment_; }
const UserInfo* URI::getUserInfo() const { return user_info_; }
std::string URI::getUsername() const {
  if (user_info_ == NULL) {
    return "";
  }
  return user_info_->getUsername();
}
std::string URI::getPassword() const {
  if (user_info_ == NULL) {
    return "";
  }
  return user_info_->getPassword();
}
bool URI::isForceQuery() const { return force_query_; }
const std::string& URI::getOpaque() const { return opaque_; }
bool URI::isOmitHost() const { return omit_host_; }

std::string URI::escapedPath() const { return escape(path_, Encoding::Path); }
