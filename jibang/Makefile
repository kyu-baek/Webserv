NAME		=	webserv
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 -pedantic -g3 -fsanitize=address
SRCS		=	main.cpp \
				webserver/config/configParser/Config.cpp \
				webserver/config/configParser/BaseServer.cpp \
				webserver/client/request/Request.cpp \
				webserver/client/request/Uri.cpp \
				webserver/WebServer.cpp \
				webserver/connection/Connection.cpp \
				webserver/client/Client.cpp \
				webserver/multiplex/Multiplex.cpp \


OBJS		=	$(SRCS:.cpp=.o)

all	:	$(NAME)

$(NAME)	: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@printf "=> $(NAME) created.\n"


%.o	:	%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean	:
	@rm -rf $(OBJS)
	@printf "=> $(NAME) object files removed.\n"

fclean	:	clean
	@rm -rf $(NAME)
	@printf "=> $(NAME) removed.\n"

re	:	fclean all

.PHONY : all clean fclean re
