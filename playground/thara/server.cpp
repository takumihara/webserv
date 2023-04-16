#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <iostream>

#define PORT 80

// listenのqueueのsizeを0にしても2個目のクライアントがconnectできたのなぜ？
// -> 同時に接続リクエストが来た時。connectが完了したら、queueからは消える。

typedef std::vector<int>::iterator iterator;
typedef std::vector<int>::const_iterator const_iterator;

int main() {

	int socket_fd;
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
	// fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	std::vector<int> socks;
	while (1) {
		printf("one loop\n");
		socks.push_back(accept(socket_fd, (struct sockaddr *) &add, (socklen_t *)&addlen));
		if (socks.back() == -1) {
			printf("accept error\n");
			return 1;
		}
		fcntl(socks.back(), F_SETFL, O_NONBLOCK);

		char response[100];
		memset(response, 0, 100);
		int size;
		sleep(1);
		for (iterator itr = socks.begin(); itr != socks.end(); itr++) {
			if ((size = read(*itr, response, 100)) == -1)
				continue;
			if (std::string(response) == "close") {
				close(*itr);
				socks.erase(itr);
			} else {
				std::cout << response << std::endl;
				std::cout << size << std::endl;
			}
		}
	}
}
