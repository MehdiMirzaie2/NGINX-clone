#	o-----------------o
#	|    COLOURS      |
#	o-----------------o

RESET			:= 	\033[0m
GREEN 			:= 	\033[38;5;46m
WHITE 			:= 	\033[38;5;15m
GREY 			:= 	\033[38;5;8m
ORANGE 			:= 	\033[38;5;202m
RED 			:= 	\033[38;5;160m

#	o-----------------o
#	|    VARIABLES    |
#	o-----------------o

NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address -g #-D _PRINT_
LDFLAGS = -fsanitize=address

#CFLAGS = -Wall -Wextra -Werror -g -std=c++98 #-D _PRINT_

#	o-----------------o
#	|    INCLUDES     |
#	o-----------------o

INC = inc

#	o-----------------o
#	|     SOURCES     |
#	o-----------------o

SRC_FILES = \
	ServerManager.cpp \
	Server.cpp \
	Client.cpp \
	Message.cpp \
	HTTPRequest.cpp \
	HTTPResponse.cpp \
	cgi.cpp \
	ConfigParser.cpp \
	Location.cpp \
	Utils.cpp \
	MIME.cpp \
	main.cpp

SRC_DIR = src

SRCS = $(addprefix $(SRC_DIR)/, $(SRC_FILES))

OBJ_DIR = obj

OBJ_RAW = $(SRC_FILES:.cpp=.o)

OBJS = $(addprefix $(OBJ_DIR)/, $(OBJ_RAW))

#	o-----------------o
#	|      RULES      |
#	o-----------------o

all: $(OBJ_DIR) $(NAME)

$(NAME): $(OBJS)
	@echo "$(GREY)Compiling...				$<"
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "$(GREEN)----------------------------------------------------"
	$(CC) $(CFLAGS) -I $(INC) -c $< -o $@
	@echo "Executable				./$(NAME) $(RESET)"
# @echo "Making $<"

$(OBJ_DIR):
	mkdir -p $@

clean:
	@echo "$(RED)----------------------------------------------------"
	rm -f $(OBJS)
	rm -rf $(OBJ_DIR)
	@echo "$(GRAY)REMOVED O-FILES $(RESET)"

fclean: clean
	@echo "$(RED)----------------------------------------------------"
	rm -f $(NAME)
	@echo "$(GRAY)REMOVED EXECUTABLE FILE $(RESET)"

re: fclean all

.PHONY: all clean fclean re
