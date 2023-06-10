#include <gtest/gtest.h>

#include "../src/Config/Config.hpp"
#include "../src/Config/Parser.hpp"
#include "../src/Config/validation.h"
#include "../src/EventManager.hpp"
#include "../src/HttpRequest.hpp"
#include "../src/HttpResponse.hpp"
#include "mock/MockReadCloser.hpp"

TEST(Routing, basic) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, basic_no_such_server) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: no_such_server\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, multi_loc_first) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, multi_loc_second) {
  IReadCloser *rc = new MockReadCloser("GET /api/hello HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[1]);
}

TEST(Routing, multi_loc_third) {
  IReadCloser *rc = new MockReadCloser("GET /hello HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[2]);
}

TEST(Routing, multi_loc_no_match) {
  IReadCloser *rc = new MockReadCloser("GET /no_such HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, multi_serv_first) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, multi_serv_second) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: server2\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[1].location_confs_[0]);
}

TEST(Routing, multi_serv_no_such_server) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: no_such_server\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, multi_serv_loc_1S_1L) {
  IReadCloser *rc = new MockReadCloser("GET /serv HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, multi_serv_loc_1S_2L) {
  IReadCloser *rc = new MockReadCloser("GET /api HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[1]);
}

TEST(Routing, multi_serv_loc_1S_noL) {
  IReadCloser *rc = new MockReadCloser("GET /world HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, multi_serv_loc_2S_1L) {
  IReadCloser *rc = new MockReadCloser("GET /serv2 HTTP/1.1\r\nHost: server2\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[1].location_confs_[0]);
}

TEST(Routing, multi_serv_loc_2S_3L) {
  IReadCloser *rc = new MockReadCloser("GET /hello2 HTTP/1.1\r\nHost: server2\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[1].location_confs_[2]);
}

TEST(Routing, multi_serv_loc_2S_noL) {
  IReadCloser *rc = new MockReadCloser("GET /nosuch2 HTTP/1.1\r\nHost: server2\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[1].location_confs_[0]);
}

TEST(Routing, multi_serv_loc_noS_2L) {
  IReadCloser *rc = new MockReadCloser("GET /api HTTP/1.1\r\nHost: no_such\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[1]);
}

TEST(Routing, multi_serv_loc_noS_noL) {
  IReadCloser *rc = new MockReadCloser("GET /nosuch HTTP/1.1\r\nHost: no_such\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/multiple_serv_loc.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, limit_method_get) {
  IReadCloser *rc = new MockReadCloser("GET / HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic_limit_method.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, limit_method_post) {
  IReadCloser *rc = new MockReadCloser("POST / HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic_limit_method.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[1]);
}

TEST(Routing, limit_method_noM) {
  IReadCloser *rc = new MockReadCloser("DELETE / HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic_limit_method.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, allowed_ext_cgi) {
  IReadCloser *rc = new MockReadCloser("GET /a.cgi HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic_allowed_ext.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, allowed_ext_py) {
  IReadCloser *rc = new MockReadCloser("GET /a.py HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic_allowed_ext.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[1]);
}

TEST(Routing, allowed_ext_noExt) {
  IReadCloser *rc = new MockReadCloser("GET /a.php HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/basic_allowed_ext.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, allowed_ext_pathLen) {
  IReadCloser *rc = new MockReadCloser("GET /a.py HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/allowed_ext_pathLen.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[1]);
}

TEST(Routing, allowed_ext_pathLen_api) {
  IReadCloser *rc = new MockReadCloser("GET /api/a.py HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/allowed_ext_pathLen.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[2]);
}

TEST(Routing, allowed_ext_pathLen_noPath) {
  IReadCloser *rc = new MockReadCloser("GET /hello/a.py HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/allowed_ext_pathLen.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[1]);
}

TEST(Routing, limit_method_pathLen) {
  IReadCloser *rc = new MockReadCloser("POST /path2 HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/limit_method_pathLen.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[2]);
}

TEST(Routing, limit_method_pathLen_path1) {
  IReadCloser *rc = new MockReadCloser("POST /path1/hello HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/limit_method_pathLen.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[1]);
}

TEST(Routing, limit_method_pathLen_noPath) {
  IReadCloser *rc = new MockReadCloser("POST /api/hello HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/limit_method_pathLen.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}

TEST(Routing, limit_method_pathLen_noM) {
  IReadCloser *rc = new MockReadCloser("DELETE /path1 HTTP/1.1\r\nHost: server1\r\n\r\n");
  Parser parser;
  Config conf = parser.parse("test/www/config/limit_method_pathLen.conf");
  conf.makePortServConfMap();
  if (!isServernameDuplicate(conf)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
  HttpRequest req(0, &conf);
  HttpRequest::readRequest(req, rc);
  ServerConf *serv_conf = conf.getServerConf(req.getHost().port, req.getHost().uri_host);
  LocationConf *loc_conf = serv_conf->getLocationConf(&req);
  EXPECT_TRUE(*loc_conf == conf.server_confs_[0].location_confs_[0]);
}