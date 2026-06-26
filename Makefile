CXX = c++

NAME = webserv

CXXFLAGS = -Wall -Wextra -Werror  -std=c++98 -g -fsanitize=address

SRC =	main.cpp \
		parse_config/src/Fill.cpp \
		parse_config/src/FillLocation.cpp \
		parse_config/src/FillServer.cpp \
		parse_config/src/parse.cpp \
		parse_config/src/statemachine.cpp \
		parse_request/src/HttpRequest.cpp \
		build_response/HttpResponse.cpp \
		event_loop/loopTools.cpp \
		route/RouteResp.cpp

INC =	parse_config/inc/Fill.hpp \
		parse_config/inc/FillLocation.hpp \
		parse_config/inc/FillServer.hpp \
		parse_config/inc/parse.hpp \
		parse_request/inc/HttpRequest.hpp \
		build_response/HttpResponse.hpp \
		event_loop/loopTools.hpp \
		route/RouteResp.hpp

OBJ = ${SRC:.cpp=.o}

all: $(NAME)

$(NAME) : $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	
%.o : %.cpp $(INC)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	@rm -rf $(OBJ)

fclean : clean
	@rm -rf $(NAME)

re : fclean all

ac : all clean