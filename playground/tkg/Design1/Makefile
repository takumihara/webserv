CXXFLAGS	= -Wall -Werror -Wextra -std=c++98
SRC_DIR = src
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

OBJ_DIR = obj
SERVER_OBJS	= $(SERVER_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

DEPENDS = $(SERVER_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.d)

SRC_DIRS := $(shell find $(SRC_DIR) -type d -print)
INCLUDE		=	-I $(SRC_DIR) \
				$(addprefix -I ,$(SRC_DIRS))

gtestdir	=	./test/gtest
gtest		=	$(gtestdir)/gtest $(gtestdir)/googletest-release-1.11.0

testdir = ./test

# todo(thara): rm
CXXFLAGS += -D DEBUG
ifdef DEBUG
	CXXFLAGS += -D DEBUG
endif

all: server client

server: $(SERVER_OBJS)
	clang++ $(SERVER_OBJS) -o server

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@);
	clang++ $(CXXFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@


client:
	clang++ $(CXXFLAGS) $(INCLUDE) src/client/client.cpp src/helper.cpp -o client


clean:
	$(RM) $(SERVER_OBJS) $(DEPENDS) 

fclean:
	$(RM) $(SERVER_OBJS) client server test.log tester


re: fclean all

-include $(DEPENDS)

debug:
	make DEBUG=1
fclean: clean

.PHONY: clean fclean re server client

$(gtest):
	curl -OL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
	tar -xvzf release-1.11.0.tar.gz googletest-release-1.11.0
	$(RM) -rf release-1.11.0.tar.gz
	python3 googletest-release-1.11.0/googletest/scripts/fuse_gtest_files.py $(gtestdir)
	mv googletest-release-1.11.0 $(gtestdir)

test_compile = clang++ -std=c++11 \
	$(testdir)/*.cpp $(gtestdir)/googletest-release-1.11.0/googletest/src/gtest_main.cc $(gtestdir)/gtest/gtest-all.cc \
	-I$(gtestdir) -I$(includes) -lpthread -o tester
.PHONY: test
test: server $(gtest)
	$(test_compile)
	cp server server_binary
	./server_binary > test.log &
	-./tester && pkill server_binary
	@rm server_binary


#Config Unit Test start
CONFTEST_SRCS =	$(SRC_DIR)/Config/Config.cpp \
				$(SRC_DIR)/Config/validation.cpp \
				$(SRC_DIR)/Config/Parser.cpp \
				$(SRC_DIR)/const.cpp \
				$(testdir)/Config/test.cpp


conftest: $(gtest)
	clang++ -D CONFTEST -std=c++11 \
	$(CONFTEST_SRCS) $(INCLUDE) $(gtestdir)/googletest-release-1.11.0/googletest/src/gtest_main.cc $(gtestdir)/gtest/gtest-all.cc \
	-I$(gtestdir) -lpthread -o conftest
	-./conftest
	@rm conftest

unit_conf: 
	clang++  -std=c++11 $(CONFTEST_SRCS) $(INCLUDE) $(testdir)/Config/main.cpp -o ./test/Config/a.out

.PHONY: conftest
	