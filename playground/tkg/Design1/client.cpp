// https://code-bug.net/entry/2019/09/23/210557/
// https://www.geekpage.jp/programming/winsock/getaddrinfo-2.php

// 疑問
// なぜlocalhostに接続が出来る?
// なぜgoogle.comに接続が出来ない -> !connet()ではなくconect() !=
// -1でちゃんと動く(繋がってないのに繋がったと出力されて繋がってるのに繋がってないと出力されていた)

#include <libc.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>
#include <string>

#include "helper.hpp"

std::string getRequest(const std::string &arg);

int main(int argc, char **argv) {
  std::string hostname = "localhost";
  // can be "http"

  std::string service = argv[1];
  struct addrinfo hints, *res;
  int err;
  int sock;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  // 名前解決の方法を指定
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hostname.c_str(), service.c_str(), &hints, &res)) != 0) {
    printf("error %d\n", err);
    return 1;
  }
  void *ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
  char addr_buf[64];
  inet_ntop(res->ai_family, ptr, addr_buf, sizeof(addr_buf));
  printf("ADDRESS: %s\n", addr_buf);

  sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sock == -1) {
    perror("socket");
    return 1;
  }

  if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
    perror("connect");
    return 1;
  } else
    printf("connection success!\n");

  for (int i = 2; i < argc; i++) {
    std::string request = getRequest(argv[i]);
    int write_res = sendto(sock, request.c_str(), request.size(), 0, NULL, 0);
    if (write_res == -1) {
      perror("write");
    } else {
      std::cout << "request sent: "
                << "'" << escape(request) << "'"
                << "(fd:" << sock << "): '"
                << " (size:" << write_res << ")" << std::endl;
    }
    char response[1000];
    memset(response, 0, 1000);
    ssize_t res = read(sock, response, 1000);
    if (res == -1) {
      perror("read");
      exit(1);
    }
    std::cout << "response received"
              << "(fd:" << sock << "): '" << escape(response) << "' (size: " << res << ")" << std::endl;
    sleep(3);
  }

  freeaddrinfo(res);
  close(sock);

  return 0;
}

/*
4␍␊            (chunk size is four octets)
Wiki           (four octets of data)
␍␊             (end of chunk)

7␍␊            (chunk size is seven octets)
pedia i        (seven octets of data)
␍␊             (end of chunk)

B␍␊            (chunk size is eleven octets)
n ␍␊chunks.    (eleven octets of data)
␍␊             (end of chunk)

0␍␊            (chunk size is zero octets, no more chunks)
␍␊
*/

std::string getRequest(const std::string &arg) {
  std::string request = "";
  if (arg == "chunked") {
    request += "POST /index.html HTTP/1.1\r\n";
    request += "Host: localhost\r\n";
    request += "Transfer-Encoding: chunked\r\n ";
    request += "Content-Type: text/plain\r\n";
    request += "\r\n";
    request += "4\r\nWiki\r\n7\r\npedia i\r\nB\r\nn \r\nchunks.\r\n0\r\n\r\n";
    request += "\r\n";
  } else {
    request += "POST /index.html HTTP/1.1\r\n";
    // request += "POST /index.html HTTP/1.1\r\n";
    request += "Host: localhost\r\n";
    request += "Content-Type: text/plain\r\n";
    request += "Content-Length: 5\r\n";
    request += "\r\n";
    request += arg;
    request += "\r\n";
  }
  return request;
}
