#include <gtest/gtest.h>

#include "../src/Config/Config.hpp"
#include "../src/EventManager.hpp"
#include "../src/HttpRequest.hpp"
#include "mock/MockReadCloser.hpp"

TEST(Request, Get) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  EventManager em = EventManager();
  Config conf;
  HttpRequest req(0, 0, conf);
  bool finished = HttpRequest::readRequest(req, em, rc);

  ASSERT_TRUE(finished);
  ASSERT_EQ(req.getRequestTarget().absolute_path, "/");
}
