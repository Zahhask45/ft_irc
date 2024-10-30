_END		=	\e[0m
_BOLD		=	\e[1m
_UNDER		=	\e[4m
_REV		=	\e[7m

_WHITE		=	\e[1;30m
_RED		=	\e[1;31m
_GREEN		=	\e[1;32m
_YELLOW		=	\e[1;33m
_BLUE		=	\e[1;34m
_PURPLE		=	\e[1;35m
_CYAN		=	\e[1;36m
_WHITE		=	\e[1;37m

_GONE		=	\e[2K\r

#CXX					= clang++ ## em casa
CXX					= c++ ## na escola
CXX_WARNINGS 		= -Wall -Wextra -Werror
CXX_RESTRICTION 	= -std=c++98 -pedantic-errors
CXX_DEPENDENCIES 	= -MMD -MP -MF $(DEPS_DIR)/$*.d
CXXFLAGS 			= $(CXX_WARNINGS) $(CXX_RESTRICTION) $(CXX_DEPENDENCIES)
DEBUG				= -ggdb -fno-limit-debug-info -fsanitize=address

NAME		=	ircserv

SRC_DIR		=	./src/
OBJ_DIR		=	./obj/
DEPS_DIR	=	./deps/

INCLUDE		=	-I ./inc/

SRCS		=	main.cpp \
				Server.cpp \
				Client.cpp \
				Channel.cpp \
				commands.cpp \
				cmdAuth.cpp \
				cmdInvite.cpp \
				cmdKick.cpp \
				cmdMode.cpp \
				cmdTopic.cpp \
				Bot.cpp \

SRC			=	$(addprefix $(SRC_DIR), $(SRCS))
OBJS		=	$(SRCS:.cpp=.o)
OBJ			=	$(addprefix $(OBJ_DIR), $(OBJS))
DEPS		=	$(SRCS:.cpp=.d)
DEP			=	$(addprefix $(DEPS_DIR), $(DEPS))

all:	$(NAME)

$(NAME): $(OBJ)
	printf "$(_GONE) $(_GREEN) All files compiled into $(OBJ_DIR), $(DEPS_DIR) $(_END)\n"
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) $(INCLUDE)
	
	printf "$(_GONE) $(_GREEN) Executable $(NAME) created $(_END)\n"

$(OBJ): | $(OBJ_DIR) $(DEPS_DIR)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	printf "$(_GONE) $(_YELLOW) Compiling $< $(_END)"
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(DEPS_DIR):
	mkdir -p $(DEPS_DIR)

clean:
	rm -rf $(OBJ_DIR) $(DEPS_DIR)
	printf "$(_GONE) $(_RED) $(OBJ_DIR) and $(DEPS_DIR) have been deleted $(_END)\n"

fclean: clean
	rm -rf $(NAME)
	printf "$(_GONE) $(_RED) $(NAME) has been deleted $(_END)\n"

re: fclean all

# run: valgrind
run: debug
	./$(NAME) 6667 banana123

debug: CXXFLAGS += $(DEBUG)
debug: re

valgrind: CXXFLAGS += -ggdb
valgrind: re
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind-out.txt ./$(NAME) 6667 banana123

.SILENT:

.PHONY: all clean fclean re run debug valgrind

-include $(DEP)