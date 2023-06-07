#include "../src/URI/URI.hpp"

#include <gtest/gtest.h>

// URI

TEST(URIParse, URIGeneral) {
  URI *uri = URI::parse("http://user:password@www.example.com/info?query#fragment");

  ASSERT_EQ(uri->getScheme(), "http");
  ASSERT_EQ(uri->getHost(), "www.example.com");
  ASSERT_EQ(uri->getPath(), "/info");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "fragment");
  ASSERT_EQ(uri->getUsername(), "user");
  ASSERT_EQ(uri->getPassword(), "password");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// URI -> "//" authority path-abempty
TEST(URIParse, SchemaOnly) {
  URI *uri = URI::parse("schema://");

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, EmptyHost) {
  URI *uri = URI::parse("schema:///");

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "/");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, PathSlash) {
  URI *uri = URI::parse("http://example.com//my/path");

  ASSERT_EQ(uri->getScheme(), "http");
  ASSERT_EQ(uri->getHost(), "example.com");
  ASSERT_EQ(uri->getPath(), "//my/path");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, HostWithPort) {
  URI *uri = URI::parse("https://localhost:8080");

  ASSERT_EQ(uri->getScheme(), "https");
  ASSERT_EQ(uri->getHost(), "localhost:8080");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, HostWithUser) {
  URI *uri = URI::parse("https://user@localhost");

  ASSERT_EQ(uri->getScheme(), "https");
  ASSERT_EQ(uri->getHost(), "localhost");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "user");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, HostWithUserAndPassword) {
  URI *uri = URI::parse("https://user:password@localhost");

  ASSERT_EQ(uri->getScheme(), "https");
  ASSERT_EQ(uri->getHost(), "localhost");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "user");
  ASSERT_EQ(uri->getPassword(), "password");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, EmptyHostWithUserAndPassword) {
  URI *uri = URI::parse("https://user:password@");

  ASSERT_EQ(uri->getScheme(), "https");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "user");
  ASSERT_EQ(uri->getPassword(), "password");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, Escaped) {
  URI *uri = URI::parse("http://localhost/%40?%20");

  ASSERT_EQ(uri->getScheme(), "http");
  ASSERT_EQ(uri->getHost(), "localhost");
  ASSERT_EQ(uri->getPath(), "/@");
  ASSERT_EQ(uri->getQuery(), "%20");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// URI -> path-absolute

TEST(URIParse, OmitHost) {
  URI *uri = URI::parse("schema:/");

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "/");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), true);
}

// URI -> path-rootless

TEST(URIParse, Opaque) {
  URI *uri = URI::parse("https:a?query#fragment");

  ASSERT_EQ(uri->getScheme(), "https");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "fragment");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "a");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// URI -> path-empty

TEST(URIParse, URIPathEmpty) {
  URI *uri = URI::parse("https:?query#fragment");

  ASSERT_EQ(uri->getScheme(), "https");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "fragment");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// relative-ref

TEST(URIParse, RelativeGeneral) {
  URI *uri = URI::parse("//localhost:8080?query#fragment");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "localhost:8080");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "fragment");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// relative-ref -> "//" authority path-abempty

TEST(URIParse, RelativeAuthority) {
  URI *uri = URI::parse("//localhost");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "localhost");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, RelativeAuthorityPathAbs) {
  URI *uri = URI::parse("//localhost:80/path");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "localhost:80");
  ASSERT_EQ(uri->getPath(), "/path");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// im not sure about this
TEST(URIParse, DoubleSlash) {
  URI *uri = URI::parse("//");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// relative-ref -> path-absolute

TEST(URIParse, PathAbsolute) {
  URI *uri = URI::parse("/path");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "/path");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// Im not sure about this
TEST(URIParse, PathMultiSlash) {
  URI *uri = URI::parse("//////a");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "//////a");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// im not sure about this
// I feel like this path should only be "/"
TEST(URIParse, TripleSlash) {
  URI *uri = URI::parse("///");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "///");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// relative-ref -> path-noscheme

TEST(URIParse, PathNoScheme) {
  URI *uri = URI::parse("nc-seg/path");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "nc-seg/path");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, Asterisk) {
  URI *uri = URI::parse("*");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "*");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(URIParse, QuestionMark) {
  URI *uri = URI::parse("?");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), true);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// relative-ref -> path-empty

TEST(URIParse, PathEmpty) {
  URI *uri = URI::parse("");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// invalid ones

TEST(URIParse, InvalidScheme) {
  try {
    URI *uri = URI::parse("cache_object://localhost/info");
    FAIL();
  } catch (std::runtime_error &e) {
    ASSERT_STREQ(e.what(), "first path segment in URL cannot contain semi-colon");
  } catch (...) {
    FAIL();
  }
}

TEST(URIParse, InvalidSemicolon) {
  try {
    URI *uri = URI::parse(":");
    FAIL();
  } catch (std::runtime_error &e) {
    ASSERT_STREQ(e.what(), "missing protocol scheme");
  } catch (...) {
    FAIL();
  }
}

TEST(URIParse, InvalidUserInfo) {
  try {
    URI *uri = URI::parse("http://user[name@example.com/");
    FAIL();
  } catch (std::runtime_error &e) {
    ASSERT_STREQ(e.what(), "URI: invalid userinfo");
  } catch (...) {
    FAIL();
  }
}

TEST(URIParse, InvalidMultiAt) {
  try {
    URI *uri = URI::parse("http://user@name@example.com/");
    FAIL();
  } catch (std::runtime_error &e) {
    ASSERT_STREQ(e.what(), "URI: invalid host");
  } catch (...) {
    FAIL();
  }
}

TEST(URIParse, InvalidUserInfoMultiColon) {
  try {
    URI *uri = URI::parse("http://username::password@example.com/");
    FAIL();
  } catch (std::runtime_error &e) {
    ASSERT_STREQ(e.what(), "URI: invalid userinfo");
  } catch (...) {
    FAIL();
  }
}

TEST(URIParse, InvalidHost) {
  try {
    URI *uri = URI::parse("http://e[xample.com/");
    std::cout << uri->getUserInfo()->getString() << std::endl;
    FAIL();
  } catch (std::runtime_error &e) {
    ASSERT_STREQ(e.what(), "URI: invalid host");
  } catch (...) {
    FAIL();
  }
}

TEST(URIParse, InvalidEscapedHost) {
  try {
    URI *uri = URI::parse("http://%20/");
    std::cout << uri->getUserInfo()->getString() << std::endl;
    FAIL();
  } catch (std::runtime_error &e) {
    ASSERT_STREQ(e.what(), "URI: invalid host");
  } catch (...) {
    FAIL();
  }
}

// Go accepts this
TEST(URIParse, InvalidPath) {
  try {
    URI *uri = URI::parse("https://<>:8080");
    FAIL();
  } catch (std::runtime_error &e) {
    ASSERT_STREQ(e.what(), "URI: invalid host");
  } catch (...) {
    FAIL();
  }
}

/*

 if defined(R.scheme) then
    T.path      = remove_dot_segments(R.path);
 else
    if defined(R.authority) then
       T.authority = R.authority;
       T.path      = remove_dot_segments(R.path);
       T.query     = R.query;
    else
       if (R.path == "") then
          T.path = Base.path;
          if defined(R.query) then
             T.query = R.query;
          else
             T.query = Base.query;
          endif;
       else
          if (R.path starts-with "/") then
             T.path = remove_dot_segments(R.path);
          else
             T.path = merge(Base.path, R.path);
             T.path = remove_dot_segments(T.path);
          endif;
          T.query = R.query;
       endif;
       T.authority = Base.authority;
    endif;
    T.scheme = Base.scheme;
 endif;

*/

// ResolveReference

// ref is absolute-URI

TEST(ResolveReference, Absolute) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("s://u:pa@h/p?q#f");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "s");
  ASSERT_EQ(uri->getHost(), "h");
  ASSERT_EQ(uri->getPath(), "/p");
  ASSERT_EQ(uri->getQuery(), "q");
  ASSERT_EQ(uri->getFragment(), "f");
  ASSERT_EQ(uri->getUsername(), "u");
  ASSERT_EQ(uri->getPassword(), "pa");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(ResolveReference, OmitHost) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("s:/?q#f");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "s");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "/");
  ASSERT_EQ(uri->getQuery(), "q");
  ASSERT_EQ(uri->getFragment(), "f");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), true);
}

TEST(ResolveReference, Opaque) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("s:opaque/p?q#f");
  ASSERT_EQ(ref->getOpaque(), "opaque/p");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "s");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "q");
  ASSERT_EQ(uri->getFragment(), "f");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "opaque/p");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// ref is relative

TEST(ResolveReference, EmptyHost) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("//u:pa@/p?q#f");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "/p");
  ASSERT_EQ(uri->getQuery(), "q");
  ASSERT_EQ(uri->getFragment(), "f");
  ASSERT_EQ(uri->getUsername(), "u");
  ASSERT_EQ(uri->getPassword(), "pa");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(ResolveReference, EmptyUserInfo) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("//h/p?q#f");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "h");
  ASSERT_EQ(uri->getPath(), "/p");
  ASSERT_EQ(uri->getQuery(), "q");
  ASSERT_EQ(uri->getFragment(), "f");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

// todo
TEST(ResolveReference, EmptyAuthority) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("///p?q#f");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "host");
  // I really think this should be /p
  ASSERT_EQ(uri->getPath(), "///p");
  ASSERT_EQ(uri->getQuery(), "q");
  ASSERT_EQ(uri->getFragment(), "f");
  ASSERT_EQ(uri->getUsername(), "username");
  ASSERT_EQ(uri->getPassword(), "password");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(ResolveReference, Empty) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "host");
  ASSERT_EQ(uri->getPath(), "/path");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "username");
  ASSERT_EQ(uri->getPassword(), "password");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(ResolveReference, SlashSlash) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("//");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "host");
  ASSERT_EQ(uri->getPath(), "/path");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "username");
  ASSERT_EQ(uri->getPassword(), "password");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(ResolveReference, Slash) {
  URI *base = URI::parse("http://a/b/c/d;p?q");
  URI *ref = URI::parse("/");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "http");
  ASSERT_EQ(uri->getHost(), "a");
  ASSERT_EQ(uri->getPath(), "/");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_EQ(uri->getUsername(), "");
  ASSERT_EQ(uri->getPassword(), "");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(ResolveReference, ForceQuery) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("?#f");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "host");
  ASSERT_EQ(uri->getPath(), "/path");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "f");
  ASSERT_EQ(uri->getUsername(), "username");
  ASSERT_EQ(uri->getPassword(), "password");
  ASSERT_EQ(uri->isForceQuery(), true);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(ResolveReference, BaseQuery) {
  URI *base = URI::parse("schema://username:password@host/path?query#fragment");
  URI *ref = URI::parse("#f");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "host");
  ASSERT_EQ(uri->getPath(), "/path");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "f");
  ASSERT_EQ(uri->getUsername(), "username");
  ASSERT_EQ(uri->getPassword(), "password");
  ASSERT_EQ(uri->isForceQuery(), false);
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_EQ(uri->isOmitHost(), false);
}

TEST(ResolveReference, EndingWithSegment) {
  URI *base = URI::parse("schema://host/a/b/c/d");
  URI *ref = URI::parse("e/f/g");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getPath(), "/a/b/c/e/f/g");
}

TEST(ResolveReference, EndingWithSlash) {
  URI *base = URI::parse("schema://host/a/b/c/d/");
  URI *ref = URI::parse("e/f/g");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getPath(), "/a/b/c/d/e/f/g");
}

TEST(ResolveReference, EmptyPath) {
  URI *base = URI::parse("schema://host");
  URI *ref = URI::parse("e");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getPath(), "/e");
}

TEST(ResolveReference, EmptyRefPath) {
  URI *base = URI::parse("schema://host/");
  URI *ref = URI::parse("/////");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getPath(), "/////");
}

TEST(ResolveReference, MultiDoubleDot) {
  URI *base = URI::parse("schema://host/a/b/c/d");
  URI *ref = URI::parse("../..");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getPath(), "/a/");
}

TEST(ResolveReference, TooMuchDoubleDot) {
  URI *base = URI::parse("schema://host/a/b/c");
  URI *ref = URI::parse("../../../../../..");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getPath(), "/");
}

TEST(ResolveReference, TooMuchDot) {
  URI *base = URI::parse("schema://host/a/b/c");
  URI *ref = URI::parse("./././././.");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getPath(), "/a/b/");
}

TEST(ResolveReference, Dot) {
  URI *base = URI::parse("schema://host/a/b/c");
  URI *ref = URI::parse(".");

  URI *uri = base->resolveReference(*ref);

  ASSERT_EQ(uri->getPath(), "/a/b/");
}

TEST(ResolveReference, RFCCases) {
  URI *base = URI::parse("http://a/b/c/d;p?q");
  URI *ref;
  URI *uri;

  ref = URI::parse("g:h");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "g:h");
  ref = URI::parse("g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g");
  ref = URI::parse("./g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g");
  ref = URI::parse("g/");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g/");
  ref = URI::parse("/g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/g");
  ref = URI::parse("//g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://g");
  ref = URI::parse("?y");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/d;p?y");
  ref = URI::parse("g?y");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g?y");
  ref = URI::parse("#s");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/d;p?q#s");
  ref = URI::parse("g#s");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g#s");
  ref = URI::parse("g?y#s");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g?y#s");
  ref = URI::parse(";x");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/;x");
  ref = URI::parse("g;x");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g;x");
  ref = URI::parse("g;x?y#s");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g;x?y#s");
  ref = URI::parse("");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/d;p?q");
  ref = URI::parse(".");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/");
  ref = URI::parse("./");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/");
  ref = URI::parse("..");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/");
  ref = URI::parse("../");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/");
  ref = URI::parse("../g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/g");
  ref = URI::parse("../..");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/");
  ref = URI::parse("../../");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/");
  ref = URI::parse("../../g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/g");

  //  special cases
  ref = URI::parse("../../../g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/g");
  ref = URI::parse("../../../../g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/g");

  ref = URI::parse("/./g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/g");
  ref = URI::parse("/../g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/g");
  ref = URI::parse("g.");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g.");
  ref = URI::parse(".g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/.g");
  ref = URI::parse("g..");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g..");
  ref = URI::parse("..g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/..g");

  ref = URI::parse("./../g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/g");
  ref = URI::parse("./g/.");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g/");
  ref = URI::parse("g/./h");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g/h");
  ref = URI::parse("g/../h");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/h");
  ref = URI::parse("g;x=1/./y");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g;x=1/y");
  ref = URI::parse("g;x=1/../y");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/y");

  ref = URI::parse("g?y/./x");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g?y/./x");
  ref = URI::parse("g?y/../x");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g?y/../x");
  ref = URI::parse("g#s/./x");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g#s/./x");
  ref = URI::parse("g#s/../x");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http://a/b/c/g#s/../x");

  ref = URI::parse("http:g");
  uri = base->resolveReference(*ref);
  ASSERT_EQ(uri->recompose(), "http:g");
  // "http://a/b/c/g"
}
