#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc.h>
#include <netdb.h>
#include <sys/types.h>

#ifdef SERVER

#define PORT 80

int main() {
int socket_fd, accept_fd;
	struct sockaddr_in add;
	int addlen;
	char	buff[1000];
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket error\n");
		return 1;
	}
	add.sin_family = AF_INET;
	add.sin_addr.s_addr = INADDR_ANY;
	add.sin_port = htons(PORT);

	if (bind(socket_fd, (struct sockaddr *) &add, sizeof(add)) == -1) {
		printf("bind error\n");
		return 1;
	}
	if (listen(socket_fd, 3) < 0)	{
		printf("listen error\n");
		return 1;
  }
  while (1);
}

#else
// client code

int main(int argc, char **argv) {
  char *hostname = "localhost";
  char *service = "80";
  struct addrinfo hints, *res;
  int err;
  int sock;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hostname, service, &hints, &res)) != 0) {
    printf("error %d\n", err);
    return 1;
  }
  printf("%hhu\n", res->ai_addr->sa_family);
  void *ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
  char addr_buf[64];
  inet_ntop(res->ai_family, ptr, addr_buf, sizeof(addr_buf));

  sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sock == -1) {
    printf("socket\n");
    return 1;
  }

  if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
    perror("connect");
    return 1;
  }
  printf("`connect` succeeded!\n");

  while (1);

  freeaddrinfo(res);
  close(sock);

  return 0;
}


#endif