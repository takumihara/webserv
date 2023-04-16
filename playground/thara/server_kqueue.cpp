#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <set>

#define PORT 80
#define max(x, y) ((x) > (y) ? (x) : (y))

// listenのqueueのsizeを0にしても2個目のクライアントがconnectできたのなぜ？
// -> 同時に接続リクエストが来た時。connectが完了したら、queueからは消える。

typedef std::set<int>::iterator iterator;
typedef std::set<int>::const_iterator const_iterator;

int make_client_connection(int port_fd) {
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

void handle_request(std::set<int> &socks, int socket_fd) {
  char request[100];
  int size;
  bzero(request, 100);
  if ((size = read(socket_fd, request, 100)) == -1) {
    perror("read");
    exit(1);
  }
  if (size == 0) {
    std::cout << "closed fd = " << socket_fd << std::endl;
    close(socket_fd);
    socks.erase(socket_fd);
  } else {
    std::cout << "request received"
              << "(fd:" << socket_fd << "): '" << request << "'" << std::endl;
    send_response(socket_fd, request);
  }
}

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
  int kq = kqueue();
  if (kq == -1) {
    perror("kqueue");
    exit(1);
  }
  setsockopt

  std::set<int> socks;
  socks.insert(port_fd);
  std::cout << "server setup finished!" << std::endl;
  while (1) {
    struct kevent *chlist = new struct kevent[socks.size()];
    int i = 0;
    for (const_iterator itr = socks.begin(); itr != socks.end(); itr++, i++) {
      EV_SET(&chlist[i], *itr, EVFILT_READ, EV_ADD, 0, 0, 0);
    }

    struct kevent *evlist = new struct kevent[socks.size()];
    int nev = kevent(kq, chlist, socks.size(), evlist, socks.size(), NULL);
    if (nev == 0)
      continue;
    else if (nev == -1)
      perror("kevent");

    for (int i = 0; i < nev; i++) {
      if (evlist[i].ident == port_fd) {
        socks.insert(make_client_connection(port_fd));
      } else {
        handle_request(socks, evlist[i].ident);
      }
    }
  }
}
