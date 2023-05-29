CXXFLAGS	= -Wall -Werror -Wextra -std=c++98
SRC_DIR = .
SERVER_SRCS	= $(SRC_DIR)/HttpServer.cpp \
			$(SRC_DIR)/HttpRequest.cpp \
			$(SRC_DIR)/HttpResponse.cpp \
			$(SRC_DIR)/Observee/ServerSocket.cpp \
			$(SRC_DIR)/EventManager.cpp \
			$(SRC_DIR)/Observee/ConnectionSocket.cpp \
			$(SRC_DIR)/Observee/CGI.cpp \
			$(SRC_DIR)/Config/Config.cpp \
			$(SRC_DIR)/Config/Parser.cpp \
			$(SRC_DIR)/Config/validation.cpp \
			$(SRC_DIR)/helper.cpp \
			$(SRC_DIR)/const.cpp \
			$(SRC_DIR)/main.cpp

SERVER_OBJ_DIR = obj
SERVER_OBJS	= $(SERVER_SRCS:$(SRC_DIR)/%.cpp=$(SERVER_OBJ_DIR)/%.o)

DEPENDS = $(SERVER_SRCS:$(SRC_DIR)/%.cpp=$(SERVER_OBJ_DIR)/%.d)

SRC_DIRS := $(shell find $(SRC_DIR) -type d -print)
INCLUDE		=	-I $(SRC_DIR) \
				$(addprefix -I ,$(SRC_DIRS))

ifdef DEBUG
	CXXFLAGS += -D DEBUG
endif

all: server client

server: $(SERVER_OBJS) 
	clang++ $(SERVER_OBJS) -o server

$(SERVER_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@);
	clang++ $(CXXFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@


client:
	clang++ $(CXXFLAGS) client.cpp helper.cpp -o client


clean:
	$(RM) $(SERVER_OBJS) $(DEPENDS)

fclean:
	$(RM) $(SERVER_OBJS) client server


re: fclean all

-include $(DEPENDS)

debug:
	make DEBUG=1
fclean: clean

.PHONY: clean fclean re server client
