
all: server client

server:
	clang++ server_kqueue_tkg.cpp ./temp/HttpServer.cpp -o server -std=c++98

client:
	clang++ client.cpp -o client -std=c++98 -Wall -Wextra -Werror

clean:
	rm -rf server client

fclean: clean

.PHONY: clean fclean re server client