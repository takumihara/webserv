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
  ASSERT_TRUE(req.methodIs(HttpRequest::GET));
  ASSERT_EQ(req.getHost().uri_host, "localhost");
  ASSERT_EQ(req.getBody(), "");
}

TEST(Request, BodyLargerThanContentLength) {
  IReadCloser *rc = new MockReadCloser("POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 3\r\n\r\nbody");
  EventManager em = EventManager();
  Config conf;
  HttpRequest req(0, 0, conf);
  bool finished = HttpRequest::readRequest(req, em, rc);

  ASSERT_TRUE(finished);
  ASSERT_EQ(req.getBody(), "body");
}

TEST(Request, NoHostFeild) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nContent-Length: 3\r\n\r\n");
  EventManager em = EventManager();
  Config conf;
  HttpRequest req(0, 0, conf);
  try {
    HttpRequest::readRequest(req, em, rc);
    FAIL();
  } catch (HttpRequest::BadRequestException &e) {
    ASSERT_EQ(std::string(e.what()), std::string("missing host header"));
  } catch (...) {
    FAIL();
  }
}

TEST(Request, BothContentLengthAndTransferEncoding) {
  IReadCloser *rc = new MockReadCloser(
      "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 4\r\nTransfer-Encoding: chunked\r\n\r\nbody");
  EventManager em = EventManager();
  Config conf;
  HttpRequest req(0, 0, conf);
  try {
    HttpRequest::readRequest(req, em, rc);
    std::cout << "No Exception" << std::endl;
    FAIL();
  } catch (HttpRequest::BadRequestException &e) {
    ASSERT_EQ(std::string(e.what()), std::string("both content-length and transfer-encoding are present"));
  } catch (...) {
    std::cout << "Wrong Exception" << std::endl;
    FAIL();
  }
}

TEST(Request, TooBigContentLength) {
  const std::string req_str =
      "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: " + std::to_string(MiB + 1) + "\r\n\r\nbody";
  IReadCloser *rc = new MockReadCloser(req_str);
  EventManager em = EventManager();
  Config conf;
  HttpRequest req(0, 0, conf);

  try {
    HttpRequest::readRequest(req, em, rc);
    std::cout << "No Exception" << std::endl;
    FAIL();
  } catch (HttpRequest::BadRequestException &e) {
    ASSERT_EQ(std::string(e.what()), std::string("Http Request: invalid content-length"));
  } catch (...) {
    std::cout << "Wrong Exception" << std::endl;
    FAIL();
  }
}

TEST(Request, NegativeContentLength) {
  const std::string req_str = "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: -1\r\n\r\nbody";
  IReadCloser *rc = new MockReadCloser(req_str);
  EventManager em = EventManager();
  Config conf;
  HttpRequest req(0, 0, conf);

  try {
    HttpRequest::readRequest(req, em, rc);
    std::cout << "No Exception" << std::endl;
    FAIL();
  } catch (HttpRequest::BadRequestException &e) {
    ASSERT_EQ(std::string(e.what()), std::string("Http Request: invalid content-length"));
  } catch (...) {
    std::cout << "Wrong Exception" << std::endl;
    FAIL();
  }
}
