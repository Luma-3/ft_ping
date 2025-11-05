NAME = ft_ping

SRC_DIR = src/
SRC_FILES =				\
				main.c	\
				packet.c	\
				parsing.c	\
				ionet.c \
				utils.c	\

SRC = $(addprefix $(SRC_DIR), $(SRC_FILES))

OBJ_DIR = obj/
OBJ_FILES = $(SRC_FILES:.c=.o)
OBJ = $(addprefix $(OBJ_DIR), $(OBJ_FILES))

INC_DIR = inc/
INC = -I $(INC_DIR)

CC = gcc
CFLAGS = -Wall -Wextra -Werror -g3

all: $(NAME)

$(NAME): $(OBJ) 
	$(CC) $(CFLAGS) $(INC) $^ -o $@ -lm

$(OBJ_DIR)%.o : $(SRC_DIR)%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@ -lm
 
init: 
	bear -- make -B -C.

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all
