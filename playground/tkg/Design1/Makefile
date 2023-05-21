
CXXFLAGS	= -Wall -Werror -Wextra -std=c++98
SERVER_SRC	= HttpServer.cpp HttpRequest.cpp HttpResponse.cpp ServerSocket.cpp EventManager.cpp ConnectionSocket.cpp \
Config/Config.cpp Config/Parser.cpp Config/validation.cpp helper.cpp main.cpp

ifdef DEBUG
	CXXFLAGS += -D DEBUG
endif

all: server client

server:
	clang++ $(CXXFLAGS) $(SERVER_SRC) -o server

client:
	clang++ $(CXXFLAGS) client.cpp -o client

clean:
	rm -rf server client

debug:
	make DEBUG=1
fclean: clean

.PHONY: clean fclean re server client
