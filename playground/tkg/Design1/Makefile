
CXXFLAGS = -Wall -Werror -Wextra -std=c++98

ifdef DEBUG
	CXXFLAGS += -D DEBUG
endif
all: server client

server:
	clang++ $(CXXFLAGS) *.cpp -o server

client:
	clang++ $(CXXFLAGS) ../echoserver/client.cpp -o client

clean:
	rm -rf server client

debug:
	make DEBUG=1
fclean: clean

.PHONY: clean fclean re server client
