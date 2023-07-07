#include <gtest/gtest.h>
#include <libc.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fstream>
#include <iostream>
#include <string>

bool includes(const std::string &str, const std::string &substr);
std::string sendRequest(const std::string &host, const std::string &port, const std::string &method,
                        const std::string &path, const std::string &body, const std::string &headers);
std::string sendInvalidRequest(const std::string &host, const std::string &port, const std::string &method,
                               const std::string &path, const std::string &body, const std::string &headers);

TEST(E2E, CGIDoc) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/cgi-bin/cgi_DocRes.cgi?query";
  std::string body = "Body\r\n";
  std::string headers = "Host: localhost;Content-Length:6;Date: Wed, 16 Oct 2019 07:28:00 GMT";
  std::string res = sendRequest(host, port, method, path, body, headers);

  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "CGI Response \n"));
}

TEST(E2E, CGI_CWD) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/cgi-bin/cgi_getCWD.cgi?query";
  std::string body = "";
  std::string headers = "Host: localhost;Content-Length:0;Date: Wed, 16 Oct 2019 07:28:00 GMT";
  std::string res = sendRequest(host, port, method, path, body, headers);

  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "/playground/tkg/Design1/cgi-bin\n"));
}

TEST(E2E, CGILocalRedirect) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/cgi-bin/cgi_LocalRedirect.cgi?query";
  std::string body = "Body\r\n";
  std::string headers = "Host: localhost;Content-Length:6;Date: Wed, 16 Oct 2019 07:28:00 GMT";

  std::string res = sendRequest(host, port, method, path, body, headers);

  std::cerr << res << std::endl;
  ASSERT_TRUE(includes(res, "<!DOCTYPE html>"));
}

TEST(E2E, CGIClientRedirect) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/cgi-bin/cgi_ClientRedirect.cgi?query";
  std::string body = "Body\r\n";
  std::string headers = "Host: localhost;Content-Length:6;Date: Wed, 16 Oct 2019 07:28:00 GMT";

  std::string res = sendRequest(host, port, method, path, body, headers);

  std::cerr << res << std::endl;
  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "HTTP/1.1 302 Found\n"));
  ASSERT_TRUE(includes(res, "location: http://example.com"));
}

TEST(E2E, CGIClientRedirectWithDoc) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/cgi-bin/cgi_ClientRedirWithDoc.cgi?query";
  std::string body = "Body\r\n";
  std::string headers = "Host: localhost;Content-Length:6;Date: Wed, 16 Oct 2019 07:28:00 GMT";

  std::string res = sendRequest(host, port, method, path, body, headers);

  std::cerr << res << std::endl;
  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "location: http://example.com"));
  ASSERT_TRUE(includes(res, "Client Redirection With Document CGI Response"));
}

TEST(E2E, CGILocalRedirectToClientRedirect) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/cgi-bin/local.py?query";
  std::string body = "Body\r\n";
  std::string headers = "Host: localhost;Content-Length:6;Date: Wed, 16 Oct 2019 07:28:00 GMT";

  std::string res = sendRequest(host, port, method, path, body, headers);

  std::cerr << res << std::endl;
  ASSERT_TRUE(includes(res, "HTTP/1.1 302 Found\n"));
  ASSERT_TRUE(includes(res, "location: http://example.com"));
}

TEST(E2E, Get_index_html) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/html/index.html";
  std::string body = "hello";
  std::string headers = "Host: localhost;Content-Length:5";
  std::string res = sendRequest(host, port, method, path, body, headers);
  std::cerr << res << std::endl;
  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "content-type: text/html"));
  ASSERT_TRUE(includes(res, "<!DOCTYPE html>"));
}

TEST(E2E, Get_42_jpg) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/html/42.jpg";
  std::string body = "hello";
  std::string headers = "Host: localhost;Content-Length:5";
  std::string res = sendRequest(host, port, method, path, body, headers);
  std::cerr << res << std::endl;
  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "content-type: image/jpeg"));
  std::ifstream ifs("../html/42.jpg");
  std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  ASSERT_TRUE(includes(res, str));
}

TEST(E2E, Autoindex) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/Config/";
  std::string body = "hello";
  std::string headers = "Host: localhost";
  std::string res = sendRequest(host, port, method, path, body, headers);
  std::cerr << res << std::endl;
  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "content-type: text/html"));
  ASSERT_TRUE(includes(res, "<!DOCTYPE html>"));
  ASSERT_TRUE(includes(res, "<p><a href=\"http://localhost:80"));
  ASSERT_TRUE(includes(res, "/Config/con.conf\">con.conf </a><br></p>"));
}

TEST(E2E, Non_Autoindex) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/cgi-bin/";
  std::string body = "hello";
  std::string headers = "Host: localhost";
  std::string res = sendRequest(host, port, method, path, body, headers);
  std::cerr << res << std::endl;
  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "HTTP/1.1 404 Not Found"));
}

TEST(E2E, NotImplementedDefaultError) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GETS";
  std::string path = "/cgi-bin/";
  std::string body = "hello";
  std::string headers = "Host: localhost";
  std::string res = sendRequest(host, port, method, path, body, headers);
  std::cerr << res << std::endl;
  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "HTTP/1.1 501 Not Implemented"));
  ASSERT_TRUE(includes(
      res, "<p>Our apologies for the temporary inconvenience. The requested URL was not found on this server.\n</p>"));
}

TEST(E2E, duplicate_host_header) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/cgi-bin/";
  std::string body = "hello";
  std::string headers = "Host: localhost;Host: localhost";
  std::string res = sendInvalidRequest(host, port, method, path, body, headers);
  std::cerr << res << std::endl;
  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "HTTP/1.1 400 Bad Request"));
  ASSERT_TRUE(includes(
      res, "<p>Our apologies for the temporary inconvenience. The requested URL was not found on this server.\n</p>"));
}

TEST(E2E, Chunked) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/html/index.html";
  std::string body = "4\r\nWiki\r\n7\r\npedia i\r\nB\r\nn \r\nchunks.\r\n0\r\n";
  std::string headers = "Host: localhost;Transfer-Encoding:chunked, chunked     , chunked  ";

  std::string res = sendRequest(host, port, method, path, body, headers);
  std::cerr << res << std::endl;
  // ASSERT_TRUE(includes(res, "HTTP/1.1 200 OK"));
  ASSERT_TRUE(includes(res, "<!DOCTYPE html>"));
}

TEST(E2E, ObsFold) {
  std::string host = "localhost";
  std::string port = "80";
  std::string method = "GET";
  std::string path = "/html/index.html";
  std::string body = "4\r\nWiki\r\n7\r\npedia i\r\nB\r\nn \r\nchunks.\r\n0\r\n";
  std::string headers = "Host: localhost;SomeHeader: SomeValue  \r\n continuous value";

  std::string res = sendRequest(host, port, method, path, body, headers);

  std::cerr << res << std::endl;

  EXPECT_TRUE(includes(res, "HTTP/1.1 400 Bad Request"));
}

bool includes(const std::string &str, const std::string &substr) { return str.find(substr) != std::string::npos; }

std::vector<std::string> extractLines(const std::string &data) {
  std::vector<std::string> lines;
  std::string str;
  std::stringstream ss(data);
  while (getline(ss, str)) {
    lines.push_back(str);
  }
  return lines;
}

std::string sendRequest(const std::string &host, const std::string &port, const std::string &method,
                        const std::string &path, const std::string &body, const std::string &headers) {
  char *argv[100];
  argv[0] = const_cast<char *>("test/client.py");
  argv[1] = const_cast<char *>(host.c_str());
  argv[2] = const_cast<char *>(port.c_str());
  argv[3] = const_cast<char *>(method.c_str());
  argv[4] = const_cast<char *>(path.c_str());
  argv[5] = const_cast<char *>(body.c_str());
  std::vector<std::string> lines;
  std::stringstream ss(headers);
  std::string str;
  while (getline(ss, str, ';')) {
    lines.push_back(str);
  }
  int i = 0;
  for (; i < lines.size(); i++) {
    argv[6 + i] = const_cast<char *>(lines[i].c_str());
  }
  argv[6 + i] = NULL;
  extern char **environ;
  int fd[2];
  std::string response = "";
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1) {
    throw std::runtime_error("soketpair error");
  }
  int pid = fork();
  if (pid == 0) {
    close(fd[0]);
    if (dup2(fd[1], STDIN_FILENO) == -1) {
      perror("dup2");
      close(fd[1]);
      exit(1);
    }
    if (dup2(fd[1], STDOUT_FILENO) == -1) {
      perror("dup2");
      close(fd[1]);
      exit(1);
    }
    close(fd[1]);
    if (execve(argv[0], argv, environ) == -1) {
      perror("execve");
      exit(1);
    }
  } else {
    close(fd[1]);
    int res = 0;
    char buff[10000];
    while ((res = read(fd[0], buff, 10000)) != 0) {
      buff[res] = '\0';
      response += buff;
    }
    close(fd[0]);
  }

  return response;
}

std::string sendInvalidRequest(const std::string &host, const std::string &port, const std::string &method,
                               const std::string &path, const std::string &body, const std::string &headers) {
  struct addrinfo hints, *res;
  int err;
  int sock;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  // 名前解決の方法を指定
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(host.c_str(), port.c_str(), &hints, &res)) != 0) {
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
  std::stringstream ss;
  ss << method << " " << path << " "
     << "HTTP/1.1\r\n";
  std::vector<std::string> lines;
  std::stringstream hs(headers);
  std::string str;
  if (headers != "") {
    while (getline(hs, str, ';')) {
      ss << str << "\r\n";
    }
  }
  ss << "\r\n";
  ss << body;
  char response[10000];
  std::string request = ss.str();
  int write_res = sendto(sock, request.c_str(), request.size(), 0, NULL, 0);
  if (write_res == -1) {
    perror("write");
  }
  memset(response, 0, 10000);
  ssize_t read_res = -1;
  size_t total = 0;
  while (read_res != 0) {
    read_res = read(sock, response + total, 10000 - total);
    if (read_res == -1) {
      perror("read");
      exit(1);
    }
    total += read_res;
    response[total] = '\0';
  }
  freeaddrinfo(res);
  close(sock);
  return std::string(response);
}
