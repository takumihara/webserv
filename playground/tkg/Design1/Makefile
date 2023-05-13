
CXXFLAGS = -Wall -Werror -Wextra -std=c++98

ifdef DEBUG
	CXXFLAGS += -D DEBUG
endif
all: server client

server:
	clang++ $(CXXFLAGS) ServerSocket.cpp EventManager.cpp ConnectionSocket.cpp main.cpp -o server

client:
	clang++ $(CXXFLAGS) client.cpp -o client

clean:
	rm -rf server client

debug:
	make DEBUG=1
fclean: clean

.PHONY: clean fclean re server client
