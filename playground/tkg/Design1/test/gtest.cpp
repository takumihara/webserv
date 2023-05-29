#include <gtest/gtest.h>
#include <libc.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>
#include <string>

bool includes(const std::string &str, const std::string &substr);
std::string sendRequest(const std::string &request);
std::string CGIRequest();
std::string GetRequest();
std::string ChunkedRequest();

// TEST(E2E, CGI) {
//   std::string res = sendRequest(CGIRequest());

//   EXPECT_EQ(res, std::string("GET"));
// }

TEST(E2E, Get) {
  std::string res = sendRequest(GetRequest());

  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "<!DOCTYPE html>"));
}

TEST(E2E, Chunked) {
  std::string res = sendRequest(ChunkedRequest());

  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "<!DOCTYPE html>"));
}

bool includes(const std::string &str, const std::string &substr) { return str.find(substr) != std::string::npos; }

std::string sendRequest(const std::string &request) {
  std::string hostname = "localhost";
  std::string service = "80";

  struct addrinfo hints, *res;
  int err;
  int sock;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hostname.c_str(), service.c_str(), &hints, &res)) != 0) {
    printf("error %d\n", err);
    return "";
  }
  void *ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
  char addr_buf[64];
  inet_ntop(res->ai_family, ptr, addr_buf, sizeof(addr_buf));

  sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sock == -1) {
    perror("socket");
    return "";
  }

  if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
    perror("connect");
    return "";
  }

  int write_res = sendto(sock, request.c_str(), request.size(), 0, NULL, 0);
  if (write_res == -1) {
    perror("sendto");
  }

  char response[1000];
  memset(response, 0, 1000);
  if (read(sock, response, 1000) == -1) {
    perror("read");
    return "";
  }

  freeaddrinfo(res);
  close(sock);

  return std::string(response);
}

std::string GetRequest() {
  std::string request;
  request += "GET /index.html?query HTTP/1.1\r\n";
  request += "Host: localhost\r\n";
  request += "Content-Length:5\r\n";
  request += "\r\n";
  return request;
}

std::string CGIRequest() {
  std::string request;
  request += "POST /a.cgi?query HTTP/1.1\r\n";
  request += "Host: localhost\r\n";
  request += "Content-Length:5\r\n";
  request += "\r\n";
  request += "Body";
  request += "\r\n";
  return request;
}

std::string ChunkedRequest() {
  std::string request;
  request += "POST /index.html HTTP/1.1\r\n";
  request += "Host: localhost\r\n";
  request += "Transfer-Encoding: chunked\r\n";
  request += "\r\n";
  request += "4\r\nWiki\r\n7\r\npedia i\r\nB\r\nn \r\nchunks.\r\n0\r\n";
  request += "\r\n";
  return request;
}
