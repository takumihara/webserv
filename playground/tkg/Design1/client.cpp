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

int main(int argc, char **argv) {
  std::string hostname = "localhost";
  // can be "http"
  std::string service = "80";
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

  for (int i = 1; i < argc; i++) {
    std::string request = "";
    request += "POST / HTTP/1.1\r\n";
    request += "Host: localhost\r\n";
    request += "Content-Type: text/plain\r\n";
    request += "Content-Length: 5\r\n";
    request += "\r\n";
    request += argv[1];
    request += "\r\n";
    int write_res = sendto(sock, request.c_str(), request.size(), 0, NULL, 0);
    if (write_res == -1) {
      perror("write");
    } else {
      std::cout << "request sent: "
                << "'" << request << "'"
                << "(fd:" << sock << "): '"
                << " (size:" << write_res << ")" << std::endl;
    }
    char response[100];
    memset(response, 0, 100);
    ssize_t res = read(sock, response, 100);
    if (res == -1) {
      perror("read");
      exit(1);
    }
    std::cout << "response received"
              << "(fd:" << sock << "): '" << response << "' (size: " << res << ")" << std::endl;
    sleep(3);
  }

  freeaddrinfo(res);
  close(sock);

  return 0;
}
