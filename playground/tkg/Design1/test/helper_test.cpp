#include "../src/helper.hpp"

#include <gtest/gtest.h>

#include <string>

TEST(Helper, getExtension1) {
  std::string path = "/hello/world/text.txt";
  std::string ext = getExtension(path);

  ASSERT_EQ(ext, ".txt");
}

TEST(Helper, getExtension2) {
  std::string path = "/hello/world/text.txt.hello";
  std::string ext = getExtension(path);

  ASSERT_EQ(ext, ".hello");
}

TEST(Helper, getExtension3) {
  std::string path = "hello.csv/world.hello/text.txt";
  std::string ext = getExtension(path);

  ASSERT_EQ(ext, ".txt");
}

TEST(Helper, getExtension4) {
  std::string path = "hello.csv/world.hello/text.txt.";
  std::string ext = getExtension(path);

  ASSERT_EQ(ext, "");
}

TEST(Helper, getExtension5) {
  std::string path = "hello.csv/world.hello/.txt";
  std::string ext = getExtension(path);

  ASSERT_EQ(ext, "");
}