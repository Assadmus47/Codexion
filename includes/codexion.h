
#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdint.h>

/* parsing_utils.c */
int	is_valid_number(const char *str);
int is_digit(char c);
size_t	ft_str_to_size(const char *str, int *valid);
int is_valid_scheduler(const char *str);
int	parse_args(int argc, char **argv, size_t *values);

#endif