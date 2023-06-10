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
  Parser parser;
  Config conf = parser.parse("test/www/config/cache.conf");
  conf.makePortServConfMap();
  conf.cache_.initCache(&conf);
  std::string ref = readFile("test/www/error/error.html");
  EXPECT_EQ(ref, conf.cache_.error_page_paths_["/test/www/error/error.html"]);
  EXPECT_TRUE(true);
}

TEST(error, basic2) {
  Parser parser;
  Config conf = parser.parse("test/www/config/cache.conf");
  conf.makePortServConfMap();
  conf.cache_.initCache(&conf);
  std::string ref = readFile("test/www/error/500error.html");
  EXPECT_EQ(ref, conf.cache_.error_page_paths_["/test/www/error/500error.html"]);
  EXPECT_TRUE(true);
}

TEST(error, basic3) {
  Parser parser;
  Config conf = parser.parse("test/www/config/cache.conf");
  conf.makePortServConfMap();
  conf.cache_.initCache(&conf);
  std::string ref = readFile("test/www/error/403error.html");
  EXPECT_EQ(ref, conf.cache_.error_page_paths_["/test/www/error/403error.html"]);
  EXPECT_TRUE(true);
}

TEST(error, basic4) {
  Parser parser;
  Config conf = parser.parse("test/www/config/cache.conf");
  conf.makePortServConfMap();
  conf.cache_.initCache(&conf);
  EXPECT_TRUE(conf.cache_.error_page_paths_.find("/test/www/error/no_such_error2.html") ==
              conf.cache_.error_page_paths_.end());
}