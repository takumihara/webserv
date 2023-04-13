#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

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
	//fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	static int socks[100];
	static int sock_idx;
	while (1) {
		printf("one loop\n");
		if ((socks[sock_idx++] = accept(socket_fd, (struct sockaddr *) &add, (socklen_t *)&addlen)) == -1) {
			printf("accept error\n");
			return 1;
		}
		fcntl(accept_fd, F_SETFL, O_NONBLOCK);

		char response[100];
		memset(response, 0, 100);
		int size;
		sleep(1);
		for (int i = 0; i < sock_idx; i++) {
			if ((size = read(socks[i], response, 100)) == -1)
				continue;
			printf("%s\n", response);
			printf("%d\n", size);
		}		
		//close(accept_fd);
	}
}
