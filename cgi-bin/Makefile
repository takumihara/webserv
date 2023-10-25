CXXFLAGS	= -Wall -Werror -Wextra
SRC_DIR = script
SRC = $(wildcard $(SRC_DIR)/*.cpp)

CGI_PROG    = $(SRC:$(SRC_DIR)/%.cpp=%.cgi)

all: $(CGI_PROG)


$(CGI_PROG): %.cgi: $(SRC_DIR)/%.cpp
	clang++ $(CXXFLAGS)  $< -o $@

clean:

fclean: clean
	$(RM) -r $(CGI_PROG)

re: fclean all
