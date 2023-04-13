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

int main() {
  char *hostname = "localhost";
  // can be "http"
  char *service = "80";
  struct addrinfo hints, *res;
  int err;
  int sock;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  // 名前解決の方法を指定
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hostname, service, &hints, &res)) != 0) {
    printf("error %d\n", err);
    return 1;
  }
  printf("%hhu\n", res->ai_addr->sa_family);
  void *ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
  char addr_buf[64];
  inet_ntop(res->ai_family, ptr, addr_buf, sizeof(addr_buf));
  printf("ADDRESS: %s\n", addr_buf);

  sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sock == -1) {
    printf("socket\n");
    return 1;
  }

  if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
    perror("");
    printf("error connect\n");
    return 1;
  } else
    printf("connection success!\n");

  sleep(1);

  char *request = "message from client!";
  int write_res = sendto(sock, request, strlen(request), 0, (struct sockaddr *)&res, sizeof(res));
  if (write_res == -1) {
    perror("write");
  } else {
    printf("%d\n", write_res);
  }

  char response[100];
  memset(response, 0, 100);
  int size = read(sock, response, 100);
  printf("%s\n", response);
  printf("%d \n", size);

  // while (1);

  freeaddrinfo(res);
  close(sock);

  return 0;
}
