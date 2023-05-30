#include "EventManager.hpp"
#include "HttpServer.hpp"
#include "debug.hpp"

int main(int argc, char **argv) {
  HttpServer server;
  if (argc == 2) {
    server = HttpServer(argv[1]);
  }
  server.start();
  return 1;
}