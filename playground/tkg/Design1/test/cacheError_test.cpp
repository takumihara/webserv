#include <gtest/gtest.h>

#include <fstream>

#include "../src/Config/Cache.hpp"
#include "../src/Config/Config.hpp"
#include "../src/Config/Parser.hpp"
#include "../src/Config/validation.h"
#include "../src/EventManager.hpp"
#include "../src/HttpRequest.hpp"
#include "../src/HttpResponse.hpp"
#include "mock/MockReadCloser.hpp"

static std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("conf file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

TEST(error, basic) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/cache.conf");
  conf.makePortServConfMap();
  conf.cache_.initStatusErrorPageMap(&conf);
  std::string ref = readFile("test/www/error/error.html");
  EXPECT_EQ(ref, *conf.cache_.status_errorPage_map_["400/test/www/error/error.html"]);
  EXPECT_TRUE(true);
}

TEST(error, basic2) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/cache.conf");
  conf.makePortServConfMap();
  conf.cache_.initStatusErrorPageMap(&conf);
  std::string ref = readFile("test/www/error/500error.html");
  EXPECT_EQ(ref, *conf.cache_.status_errorPage_map_["500/test/www/error/500error.html"]);
  EXPECT_TRUE(true);
}

TEST(error, basic3) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/cache.conf");
  conf.makePortServConfMap();
  conf.cache_.initStatusErrorPageMap(&conf);
  std::string ref = readFile("test/www/error/403error.html");
  EXPECT_EQ(ref, *conf.cache_.status_errorPage_map_["403/test/www/error/403error.html"]);
  EXPECT_TRUE(true);
}

TEST(error, basic4) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/cache.conf");
  conf.makePortServConfMap();
  conf.cache_.initStatusErrorPageMap(&conf);
  EXPECT_TRUE(conf.cache_.status_errorPage_map_["402/test/www/error/error2.html"] == NULL);
}