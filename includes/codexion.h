
#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdint.h>
# include <pthread.h>

/* parsing_utils.c */
int	is_valid_number(const char *str);
int is_digit(char c);
size_t	ft_str_to_size(const char *str, int *valid);
int is_valid_scheduler(const char *str);
int	parse_args(int argc, char **argv, size_t *values);
/* time_utils.c */
size_t	get_timestamp_ms(void);

typedef struct s_simulation	t_simulation;

/* logging.c */
void    log_message(t_simulation *sim, int coder_id, char *message);
/* coder.c */
void	*coder_routine(void *arg);

typedef enum e_state
{
	COMPILING,
	DEBUGGING,
	REFACTORING
}	t_state;

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef	struct	s_dongle
{
	int				is_taken;
	size_t			timestamp;
	pthread_mutex_t	mutex;
}	t_dongle;


typedef	struct	s_coder
{
	int			id;
	t_state		coder_status;
	size_t		nb_compiles;
	size_t		last_compile_start;
	t_dongle	*left_dongle;
	t_dongle	*right_dongle;
	t_simulation	*sim;
	pthread_t		coder_thread;
}	t_coder;


typedef	struct	s_simulation
{
	size_t			number_of_coders;
	size_t			time_to_burnout;
	size_t			time_to_compile;
	size_t			time_to_debug;
	size_t			time_to_refactor;
	size_t			number_of_compiles_required;
	size_t			dongle_cooldown;
	t_scheduler		scheduler;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_mutex_t	log_mutex;
	int 			flag;
	pthread_mutex_t flag_mutex;
}	t_simulation;

#endif