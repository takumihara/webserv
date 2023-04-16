
all: server client

server:
	clang++ server_select.cpp -o server -std=c++98

client:
	clang++ client.cpp -o client -std=c++98 -Wall -Wextra -Werror

clean:
	rm -rf server client

fclean: clean

.PHONY: clean fclean re server client