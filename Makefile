CXX = c++

NAME = webserv

CXXFLAGS = -Wall -Wextra -Werror  -std=c++98 #-fsanitize=address

SRC = main.cpp tokenz/parse.cpp srcs/FillLocation.cpp srcs/FillServer.cpp

OBJ = ${SRC:.cpp=.o}

all: $(NAME)

$(NAME) : $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	
%.o : %.cpp tokenz/parse.hpp include/FillLocation.hpp include/FillServer.hpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	@rm -rf $(OBJ)

fclean : clean
	@rm -rf $(NAME)

re : fclean all

ac : all clean