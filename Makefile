NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRCS_DIR = srcs
INCS_DIR = includes

SRCS = $(SRCS_DIR)/main.c $(SRCS_DIR)/parsing.c $(SRCS_DIR)/parsing_utils.c $(SRCS_DIR)/utils.c $(SRCS_DIR)/time_utils.c $(SRCS_DIR)/logging.c $(SRCS_DIR)/coder.c $(SRCS_DIR)/dongle.c $(SRCS_DIR)/heap.c $(SRCS_DIR)/dongle_wait.c $(SRCS_DIR)/main_utils.c $(SRCS_DIR)/monitor.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -I $(INCS_DIR) $(OBJS) -o $(NAME)

%.o: %.c $(INCS_DIR)/codexion.h Makefile
	$(CC) $(CFLAGS) -I $(INCS_DIR) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean:	clean
	rm -f $(NAME)

re:	fclean all

.PHONY:	all clean fclean re