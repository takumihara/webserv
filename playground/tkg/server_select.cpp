#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#define PORT 80
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))
#define DATA_SIZE 100000000
// listenのqueueのsizeを0にしても2個目のクライアントがconnectできたのなぜ？
// -> 同時に接続リクエストが来た時。connectが完了したら、queueからは消える。

typedef std::vector<int>::iterator iterator;
typedef std::vector<int>::const_iterator const_iterator;

int update_rd(fd_set *rd, std::vector<int> &socks) {
  int max_fd = 0;
  FD_ZERO(rd);
  for (iterator itr = socks.begin(); itr != socks.end(); itr++) {
    // std::cout << *itr << " " ;
    FD_SET(*itr, rd);
    max_fd = max(max_fd, *itr);
  }
  // std::cout << std::endl;
  return max_fd;
}

int make_client_connection(fd_set *rd, int port_fd) {
  struct sockaddr_in add;
  int addlen;
  int connection_fd = accept(port_fd, (struct sockaddr *)&add, (socklen_t *)&addlen);
  if (connection_fd == -1) {
    perror("accept");
    exit(1);
  }
  return connection_fd;
}

void send_response(int socket_fd, char *response) {
  int res = sendto(socket_fd, response, strlen(response), 0, NULL, 0);
  if (res == -1) {
    perror("write");
    exit(1);
  }
  std::cout << "response sent: "
            << "'" << response << "'"
            << " (" << res << ")" << std::endl;
}

void handle_request(std::vector<int> &socks, int i) {
  char *request = (char *)calloc(INT_MAX, 1);
  int size;
  static int cur = 0;
  int fd = open("recieve.data", O_CREAT | O_RDWR);
  bzero(request, INT_MAX);
  if ((size = read(socks[i], request, 10000)) == -1) {
    perror("read");
    exit(1);
  }
  if (size == 0) {
    std::cout << "closed fd = " << socks[i] << std::endl;
    close(socks[i]);
    iterator itr = socks.begin();
    std::advance(itr, i);
    socks.erase(itr);
    return ;
  } else {
    write(fd, request, size);
  }
  cur += size;
  close(fd);
  free(request);
}

  // send_response(socks[i], request);
  /*char request[100];
  int size;
  bzero(request, 100);
  if ((size = read(socks[i], request, 100)) == -1) {
    perror("read");
    exit(1);
  }
  if (size == 0) {
    std::cout << "closed fd = " << socks[i] << std::endl;
    close(socks[i]);
    iterator itr = socks.begin();
    std::advance(itr, i);
    socks.erase(itr);
  } else {
    std::cout << "request received"
              << "(fd:" << socks[i] << "): '" << request << "'" << std::endl;
    //send_response(socks[i], request);
  }*/


int open_port() {
  int port_fd;
  struct sockaddr_in add;
  int addlen;

  if ((port_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }
  add.sin_family = AF_INET;
  add.sin_addr.s_addr = INADDR_ANY;
  add.sin_port = htons(PORT);

  if (bind(port_fd, (struct sockaddr *)&add, sizeof(add)) == -1) {
    perror("bind");
    exit(1);
  }
  if (listen(port_fd, 0) < 0) {
    perror("listen");
    exit(1);
  }
  return port_fd;
}

int main() {
  int port_fd = open_port();
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 200000;

  std::vector<int> socks;
  socks.push_back(port_fd);
  std::cout << "server setup finished!" << std::endl;
  // fcntl(port_fd, F_SETFL, O_NONBLOCK);
  while (1) {
    fd_set rd;
    int max_fd = 1;
    max_fd = update_rd(&rd, socks);
    int ret_select = select(max_fd + 1, &rd, NULL, NULL, &tv);
    if (ret_select == -1) {
      std::cout << "select error " << std::endl;
    }
    if (ret_select == 0) {
      continue;
    } else if (ret_select == -1)
      perror("select");

    for (int i = 0; i < socks.size(); i++) {
      if (FD_ISSET(socks[i], &rd) == 0) continue;
      if (socks[i] == port_fd) {
        socks.push_back(make_client_connection(&rd, port_fd));
        // fcntl(socks.back(), F_SETFL, O_NONBLOCK);
      } else {
        handle_request(socks, i);
      }
    }
  }
}
