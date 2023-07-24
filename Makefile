NAME = webserv

OBJ_DIR = objs/

CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98
RM = rm

SRC_FILE = ConfigParser RequestParser Response Server Socket main
SRCS = $(addsuffix .cpp, $(SRC_FILE))
OBJS = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(SRC_FILE)))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ_DIR)%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -rf $(OBJ_DIR)

fclean: clean
	$(RM) -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
