#include "../src/URI/URI.hpp"

#include <gtest/gtest.h>

// URI

TEST(URIParse, URIGeneral) {
  URI *uri = URI::parse("http://www.example.com/info?query#fragment");

  ASSERT_EQ(uri->getScheme(), "http");
  ASSERT_EQ(uri->getHost(), "www.example.com");
  ASSERT_EQ(uri->getPath(), "/info");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "fragment");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

// URI -> "//" authority path-abempty
TEST(URIParse, SchemaOnly) {
  URI *uri = URI::parse("schema://");

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

TEST(URIParse, EmptyHost) {
  URI *uri = URI::parse("schema:///");

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "/");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

TEST(URIParse, PathSlash) {
  URI *uri = URI::parse("http://example.com//my/path");

  ASSERT_EQ(uri->getScheme(), "http");
  ASSERT_EQ(uri->getHost(), "example.com");
  ASSERT_EQ(uri->getPath(), "//my/path");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

TEST(URIParse, HostWithPort) {
  URI *uri = URI::parse("https://localhost:8080");

  ASSERT_EQ(uri->getScheme(), "https");
  ASSERT_EQ(uri->getHost(), "localhost:8080");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

// URI -> path-absolute

TEST(URIParse, OmitHost) {
  URI *uri = URI::parse("schema:/");

  ASSERT_EQ(uri->getScheme(), "schema");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "/");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_TRUE(uri->isOmitHost());
}

// URI -> path-rootless

TEST(URIParse, Opaque) {
  URI *uri = URI::parse("https:a?query#fragment");

  ASSERT_EQ(uri->getScheme(), "https");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "fragment");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "a");
  ASSERT_FALSE(uri->isOmitHost());
}

// URI -> path-empty

TEST(URIParse, URIPathEmpty) {
  URI *uri = URI::parse("https:?query#fragment");

  ASSERT_EQ(uri->getScheme(), "https");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "fragment");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

// relative-ref

TEST(URIParse, RelativeGeneral) {
  URI *uri = URI::parse("//localhost:8080?query#fragment");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "localhost:8080");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "query");
  ASSERT_EQ(uri->getFragment(), "fragment");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

// relative-ref -> "//" authority path-abempty

TEST(URIParse, RelativeAuthority) {
  URI *uri = URI::parse("//localhost");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "localhost");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

TEST(URIParse, RelativeAuthorityPathAbs) {
  URI *uri = URI::parse("//localhost/path");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "localhost");
  ASSERT_EQ(uri->getPath(), "/path");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

// im not sure about this
TEST(URIParse, DoubleSlash) {
  URI *uri = URI::parse("//");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

// relative-ref -> path-absolute

TEST(URIParse, PathAbsolute) {
  URI *uri = URI::parse("/path");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "/path");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

// Im not sure about this
TEST(URIParse, PathMultiSlash) {
  URI *uri = URI::parse("//////a");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "//////a");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
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
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

// relative-ref -> path-noscheme

TEST(URIParse, PathNoScheme) {
  URI *uri = URI::parse("nc-seg/path");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "nc-seg/path");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

TEST(URIParse, Asterisk) {
  URI *uri = URI::parse("*");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "*");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

TEST(URIParse, QuestionMark) {
  URI *uri = URI::parse("?");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_TRUE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
}

// relative-ref -> path-empty

TEST(URIParse, PathEmpty) {
  URI *uri = URI::parse("");

  ASSERT_EQ(uri->getScheme(), "");
  ASSERT_EQ(uri->getHost(), "");
  ASSERT_EQ(uri->getPath(), "");
  ASSERT_EQ(uri->getQuery(), "");
  ASSERT_EQ(uri->getFragment(), "");
  ASSERT_FALSE(uri->isForceQuery());
  ASSERT_EQ(uri->getOpaque(), "");
  ASSERT_FALSE(uri->isOmitHost());
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

// Go accepts this
// TEST(URIParse, InvalidPath) {
//   try {
//     URI *uri = URI::parse("https://\"<>localhost:8080");
//     FAIL();
//   } catch (std::runtime_error &e) {
//     ASSERT_STREQ(e.what(), "invalid scheme");
//   } catch (...) {
//     FAIL();
//   }
// }
