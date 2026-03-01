CC = cc
CFLAGS = -Wall -Wextra -Werror
SRC = manager.c allo.c free.c reallo.c
OBJ = $(SRC:.c=.o)
NAME = izalloc.a
HEADER = izalloc.h

all: $(NAME)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	ar rcs $(NAME) $(OBJ)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
