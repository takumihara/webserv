#include "../src/URI/encoding.hpp"

#include <gtest/gtest.h>

using namespace Encoding;

TEST(Encoding, Host) {
  std::string unescaped = "www.example.com:8080";
  std::string escaped = "www.example.com:8080";
  EXPECT_EQ(escape(unescaped, Host), escaped);
  EXPECT_EQ(unescape(escaped, Host), unescaped);

  unescaped = " -._~:/?#[]@!$&\'()*+,;=";
  escaped = " -._~:/?#[]@!$&\'()*+,;=";
  EXPECT_EQ(escape(unescaped, Host), escaped);
  EXPECT_EQ(unescape(escaped, Host), unescaped);
}

TEST(Encoding, Path) {
  std::string unescaped = "/path/to/resource";
  std::string escaped = "/path/to/resource";
  EXPECT_EQ(escape(unescaped, Path), escaped);
  EXPECT_EQ(unescape(escaped, Path), unescaped);

  unescaped = "/ ?#[]:@";
  escaped = "/%20%3F%23%5B%5D:@";
  EXPECT_EQ(escape(unescaped, Path), escaped);
  EXPECT_EQ(unescape(escaped, Path), unescaped);
}

TEST(Encoding, PathSegment) {
  std::string unescaped = "resource";
  std::string escaped = "resource";
  EXPECT_EQ(escape(unescaped, PathSegment), escaped);
  EXPECT_EQ(unescape(escaped, PathSegment), unescaped);

  unescaped = "/ ?#[]:@";
  escaped = "%2F%20%3F%23%5B%5D:@";
  EXPECT_EQ(escape(unescaped, PathSegment), escaped);
  EXPECT_EQ(unescape(escaped, PathSegment), unescaped);
}

TEST(Encoding, UserInfo) {
  std::string unescaped = "user:password";
  std::string escaped = "user:password";
  EXPECT_EQ(escape(unescaped, UserInfo), escaped);
  EXPECT_EQ(unescape(escaped, UserInfo), unescaped);

  std::string original = " :%20";
  escaped = "%20:%20";
  unescaped = " : ";
  EXPECT_EQ(escape(original, UserInfo), escaped);
  EXPECT_EQ(unescape(escaped, UserInfo), unescaped);
}

TEST(Encoding, QueryComponent) {
  std::string unescaped = "key=value";
  std::string escaped = "key=value";
  EXPECT_EQ(escape(unescaped, QueryComponent), escaped);
  EXPECT_EQ(unescape(escaped, QueryComponent), unescaped);

  unescaped = "?/ :";
  escaped = "?/+:";
  EXPECT_EQ(escape(unescaped, QueryComponent), escaped);
  EXPECT_EQ(unescape(escaped, QueryComponent), unescaped);
}

TEST(Encoding, Fragment) {
  std::string unescaped = "fragment";
  std::string escaped = "fragment";
  EXPECT_EQ(escape(unescaped, Fragment), escaped);
  EXPECT_EQ(unescape(escaped, Fragment), unescaped);

  unescaped = "#?/ :";
  escaped = "%23?/%20:";
  EXPECT_EQ(escape(unescaped, Fragment), escaped);
  EXPECT_EQ(unescape(escaped, Fragment), unescaped);
}
