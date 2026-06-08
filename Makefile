CXX = c++

NAME = webserv

CXXFLAGS = -Wall -Wextra -Werror  -std=c++98 #-fsanitize=address

SRC = main.cpp parse_config/src/Fill.cpp parse_config/src/FillLocation.cpp parse_config/src/FillServer.cpp parse_config/src/parse.cpp parse_config/src/statemachine.cpp 

OBJ = ${SRC:.cpp=.o}

all: $(NAME)

$(NAME) : $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	
%.o : %.cpp parse_config/inc/Fill.hpp parse_config/inc/FillLocation.hpp parse_config/inc/FillServer.hpp parse_config/inc/parse.hpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	@rm -rf $(OBJ)

fclean : clean
	@rm -rf $(NAME)

re : fclean all

ac : all clean