#include "EventManager.hpp"
#include "HttpServer.hpp"
#include "debug.hpp"

int main() {
  HttpServer server;
  server.start();
  return 1;
}