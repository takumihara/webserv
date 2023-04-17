#include "HttpServer.hpp"

HttpServer &HttpServer::getInstance() {
	static HttpServer server;
	return server;
}

HttpServer::HttpServer(){};


