NAME=webserv
CLIENT=client
CXXFLAGS	= -Wall -Werror -Wextra -std=c++98
SRC_DIR = src

SRC_DIRS = $(shell find $(SRC_DIR) -type d -print | grep -v "Client\|*")
SERVER_SRCS_WO_MAIN	= $(filter-out %main.cpp, $(foreach path, $(SRC_DIRS), $(wildcard $(path)/*.cpp)))
SERVER_SRCS	= $(SERVER_SRCS_WO_MAIN) $(SRC_DIR)/main.cpp

VPATH = $(shell find $(SRC_DIR) -type d -print | grep -v "Client\|*")

OBJ_DIR = obj
SERVER_OBJS    = $(addprefix $(OBJ_DIR)/, $(SERVER_SRCS:%.cpp=%.o))

DEPENDS = $(addprefix $(OBJ_DIR)/, $(notdir $(SERVER_SRCS:%.cpp=%.d)))

INCLUDE		=	$(addprefix -I ,$(VPATH))

gtestdir	=	test/gtest
gtest		=	$(gtestdir)/gtest $(gtestdir)/googletest-release-1.11.0

testdir = ./test

ifdef DEBUG
	CXXFLAGS += -D DEBUG
endif

all: $(NAME) $(CLIENT) cgi

$(NAME): $(SERVER_OBJS)
	c++ $(SERVER_OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@);
	c++ $(CXXFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@


client:
	c++ $(CXXFLAGS) $(INCLUDE) src/client/client.cpp src/helper.cpp -o client

cgi:
	make -C cgi-bin

clean:
	make clean -C cgi-bin
	$(RM) $(SERVER_OBJS) $(TEST_OBJS) $(DEPENDS)

fclean: clean
	make fclean -C cgi-bin
	$(RM) -r $(CLIENT) $(NAME) $(OBJ_DIR) test/test.log $(TEST_OBJ_DIR)

re: fclean all

-include $(DEPENDS)

debug:
	make DEBUG=1

.PHONY: clean fclean re cgi

TEST_SRC_DIRS := $(testdir) $(testdir)/mock
TEST_SRCS := $(foreach dir, $(TEST_SRC_DIRS), $(wildcard $(dir)/*.cpp)) $(SERVER_SRCS_WO_MAIN)
TEST_OBJ_DIR := $(testdir)/obj
TEST_OBJS := $(addprefix $(TEST_OBJ_DIR)/, $(TEST_SRCS:%.cpp=%.o))

VPATH += $(TEST_SRC_DIRS)

GTESTLIB += ./test/gtest/libgtest.a ./test/gtest/libgtest_main.a
GTESTFLAGS := -Ltest/gtest -lgtest -lgtest_main -lpthread

$(TEST_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@);
	$(CC) -std=c++11 -D DEBUG $(INCLUDE) -I$(gtestdir) -I$(gtestdir) -I$(includes) -MMD -MP -c $< -o $@

$(gtest):
	curl -OL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
	tar -xvzf release-1.11.0.tar.gz googletest-release-1.11.0
	$(RM) -rf release-1.11.0.tar.gz
	python3 googletest-release-1.11.0/googlemock/scripts/fuse_gmock_files.py $(gtestdir)
	mv googletest-release-1.11.0 $(gtestdir)
	(cd $(gtestdir)/googletest-release-1.11.0 && mkdir build-tmp && cd build-tmp && cmake .. && make)
	mv $(gtestdir)/googletest-release-1.11.0/build-tmp/lib/libgtest.a $(gtestdir)
	mv $(gtestdir)/googletest-release-1.11.0/build-tmp/lib/libgtest_main.a $(gtestdir)

tester: $(gtest) $(TEST_OBJS)
	$(CC) $(GTESTFLAGS) -o $@ $(TEST_OBJS) $(GTESTLIB)

.PHONY: test
test: $(NAME) tester cgi
	./configTranslater.sh
	cp webserv server_binary
	./server_binary > test/test.log 2>&1 &
	@sleep 1
	-./tester
	@-pkill server_binary
	@rm server_binary

test-%: server tester
	-./tester --gtest_filter=${@:test-%=%}
