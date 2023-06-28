#include <gtest/gtest.h>

#include "../src/Config/Config.hpp"
#include "../src/EventManager.hpp"
#include "../src/HttpException.hpp"
#include "HttpRequest.hpp"
#include "mock/MockReadCloser.hpp"

TEST(Request, Get) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n");
  Config conf;
  HttpRequest req;
  HttpRequestReader rreader(0, &conf, req, rc);
  HttpRequestReader::State state = rreader.read();

  ASSERT_EQ(state, HttpRequestReader::FinishedReading);
  ASSERT_EQ(req.request_target_->getPath(), "/");
  ASSERT_TRUE(req.methodIs(HttpRequest::GET));
  ASSERT_EQ(req.headers_.host.uri_host, "localhost");
  ASSERT_EQ(std::string(req.body_.begin(), req.body_.end()), "");
}

TEST(Request, BodyLargerThanContentLength) {
  IReadCloser *rc = new MockReadCloser("POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 3\r\n\r\nbody");
  EventManager em = EventManager();
  Config conf;
  HttpRequest req;
  HttpRequestReader rreader(0, &conf, req, rc);
  HttpRequestReader::State state = rreader.read();

  ASSERT_EQ(state, HttpRequestReader::FinishedReading);
  ASSERT_EQ(std::string(req.body_.begin(), req.body_.end()), "bod");
}

TEST(Request, NoHostFeild) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nContent-Length: 3\r\n\r\n");
  EventManager em = EventManager();
  Config conf;
  HttpRequest req;
  try {
    HttpRequestReader rreader(0, &conf, req, rc);
    HttpRequestReader::State state = rreader.read();
    FAIL();
  } catch (BadRequestException &e) {
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
  HttpRequest req;
  try {
    HttpRequestReader rreader(0, &conf, req, rc);
    HttpRequestReader::State state = rreader.read();
    std::cout << "No Exception" << std::endl;
    FAIL();
  } catch (BadRequestException &e) {
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
  HttpRequest req;

  try {
    HttpRequestReader rreader(0, &conf, req, rc);
    HttpRequestReader::State state = rreader.read();
    std::cout << "No Exception" << std::endl;
    FAIL();
  } catch (BadRequestException &e) {
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
  HttpRequest req;

  try {
    HttpRequestReader rreader(0, &conf, req, rc);
    HttpRequestReader::State state = rreader.read();
    std::cout << "No Exception" << std::endl;
    FAIL();
  } catch (BadRequestException &e) {
    ASSERT_EQ(std::string(e.what()), std::string("Http Request: invalid content-length"));
  } catch (...) {
    std::cout << "Wrong Exception" << std::endl;
    FAIL();
  }
}
