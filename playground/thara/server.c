#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 80

// listenのqueueのsizeを0にしても2個目のクライアントがconnectできたのなぜ？
// -> 同時に接続リクエストが来た時。connectが完了したら、queueからは消える。

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
	if (listen(socket_fd, 0) < 0)	{
		printf("listen error\n");
		return 1;
	}
	sleep(10);
	while (1) {
		if ((accept_fd = accept(socket_fd, (struct sockaddr *) &add, (socklen_t *)&addlen)) == -1) {
			printf("accept error\n");
			return 1;
		} 
		printf("hello\n");
		char response[100];
		memset(response, 0, 100);
		int size = read(accept_fd, response, 100);
		printf("%s\n", response);
		
		close(accept_fd);
	}
}
