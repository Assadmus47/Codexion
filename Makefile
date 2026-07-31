NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRCS_DIR = srcs
INCS_DIR = includes

SRCS = $(SRCS_DIR)/main.c $(SRCS_DIR)/parsing.c $(SRCS_DIR)/parsing_utils.c $(SRCS_DIR)/utils.c $(SRCS_DIR)/time_utils.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -I $(INCS_DIR) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -I $(INCS_DIR) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean:	clean
	rm -f $(NAME)

re:	fclean all

.PHONY:	all clean fclean re