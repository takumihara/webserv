#include <gtest/gtest.h>

#include "../src/Config/Config.hpp"
#include "../src/Config/Parser.hpp"
#include "../src/Config/validation.h"
#include "../src/EventManager.hpp"
#include "../src/HttpResponse.hpp"
#include "../src/Observee/CGI/CGI.hpp"
#include "../src/Observee/CGI/CGIInfo.hpp"
#include "../src/Observee/CGI/helper.hpp"
#include "../src/Observee/ConnectionSocket.hpp"
#include "../src/URI/URI.hpp"
#include "../src/helper.hpp"
#include "HttpRequest.hpp"
#include "mock/MockReadCloser.hpp"

// isAbsURI

TEST(CGI, isAbsURI_basic1) {
  EXPECT_TRUE(CGIValidation::isAbsURI("http://user:password@www.example.com/info?key1=val1&key2=val2#fragment"));
}

TEST(CGI, isAbsURI_basic2) { EXPECT_TRUE(CGIValidation::isAbsURI("http://example.com//my/path")); }

// isAbsPath

TEST(CGI, isAbsPath_basic1) { EXPECT_TRUE(CGIValidation::isAbsPath("/example.com//my/path")); }

TEST(CGI, isAbsPath_basic2) { EXPECT_FALSE(CGIValidation::isAbsPath("example.com/my/path")); }

TEST(CGI, isAbsPath_basic3) { EXPECT_FALSE(CGIValidation::isAbsPath("/example?com/my/path")); }

TEST(CGI, isAbsPath_basic4) { EXPECT_TRUE(CGIValidation::isAbsPath("/example&aa&$com/my/path/")); }

// CGIInfo

TEST(CGI, CGIInfo1) {
  IReadCloser *rc =
      new MockReadCloser("GET /a.cgi/hello/world HTTP/1.1\r\nHost: server1\r\nCookie: name1=val1; name2=val2\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req;
  HttpRequestReader rreader(0, &conf, req, rc);
  rreader.read();
  ServerConf *serv_conf = conf.getServerConf(req.headers_.host.port, req.headers_.host.uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  std::string path = loc_conf->common_.root_ + req.request_target_->getPath();
  std::string extension = getCGIExtension(req.request_target_->getPath());

  CGIInfo info = parseCGIInfo(path, extension, req, loc_conf);
  EXPECT_EQ(info.auth_type_, "");
  EXPECT_EQ(info.content_length_, "0");
  EXPECT_EQ(info.content_type_, "");
  EXPECT_EQ(info.gateway_interface_, "CGI/1.1");
  EXPECT_EQ(info.path_info_, "/hello/world");
  EXPECT_EQ(info.path_translated_, "/www/server1/hello/world");
  EXPECT_EQ(info.query_string_, "");
  EXPECT_EQ(info.request_method_, "GET");
  EXPECT_EQ(info.script_name_, "/www/server1/a.cgi");
  EXPECT_EQ(info.server_name_, "server1");
  EXPECT_EQ(info.server_port_, "80");
  EXPECT_EQ(info.server_protocol_, "HTTP/1.1");
  EXPECT_EQ(info.server_software_, "Webserv/1.1");
  EXPECT_EQ(info.cookie_, "name1=val1; name2=val2");
}

TEST(CGI, CGIInfo2) {
  IReadCloser *rc = new MockReadCloser(
      "GET /a.cgi/hello/world?query%40 HTTP/1.1\r\nHost: server1:90\r\nContent-Length: 5\r\n\r\nhello");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic90.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req;
  HttpRequestReader rreader(0, &conf, req, rc);
  rreader.read();
  ServerConf *serv_conf = conf.getServerConf(req.headers_.host.port, req.headers_.host.uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  std::string path = loc_conf->common_.root_ + req.request_target_->getPath();
  std::string extension = getCGIExtension(req.request_target_->getPath());

  CGIInfo info = parseCGIInfo(path, extension, req, loc_conf);
  EXPECT_EQ(info.auth_type_, "");
  EXPECT_EQ(info.content_length_, "5");
  EXPECT_EQ(info.content_type_, "");
  EXPECT_EQ(info.gateway_interface_, "CGI/1.1");
  EXPECT_EQ(info.path_info_, "/hello/world");
  EXPECT_EQ(info.path_translated_, "/www/server1/hello/world");
  EXPECT_EQ(info.query_string_, "query@");
  EXPECT_EQ(info.request_method_, "GET");
  EXPECT_EQ(info.script_name_, "/www/server1/a.cgi");
  EXPECT_EQ(info.server_name_, "server1");
  EXPECT_EQ(info.server_port_, "90");
  EXPECT_EQ(info.server_protocol_, "HTTP/1.1");
  EXPECT_EQ(info.server_software_, "Webserv/1.1");
}
