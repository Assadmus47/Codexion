/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:55:58 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/12 03:51:39 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>
# include <unistd.h>

typedef struct s_simulation	t_simulation;
typedef struct s_dongle		t_dongle;
typedef struct s_heap_node	t_heap_node;
typedef struct s_heap		t_heap;
typedef struct s_coder		t_coder;

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

/* parsing_utils.c */
int				is_valid_number(const char *str);
int				is_digit(char c);
size_t			ft_str_to_size(const char *str, int *valid);
int				is_valid_scheduler(const char *str);
int				parse_args(int argc, char **argv, size_t *values);

/* time_utils.c */
size_t			get_timestamp_ms(void);
struct timespec	get_future_timespec(size_t ms);
void			usleep_ms(size_t ms);

/* logging.c */
void			log_message(t_simulation *sim, int coder_id, char *message);

/* coder.c */
void			*coder_routine(void *arg);
void			*monitor_routine(void *arg);
int				get_simulation_flag(t_coder *coder);

/* dongle.c */
int				use_dongle(t_dongle *dongle, size_t cooldown);
void			release_dongle(t_dongle *dongle);
int				acquire_dongles(t_coder *coder);
void			release_dongles(t_coder *coder);

/* heap.c */
int				heap_init(t_heap *heap, size_t capacity);
void			heap_push(t_heap *heap, t_coder *coder, size_t priority);
t_coder			*heap_pop(t_heap *heap);
void			heap_destroy(t_heap *heap);
size_t			get_priority(t_coder *coder);

/* dongle_wait.c */
int				acquire_one_dongle(t_coder *coder, t_dongle *dongle);

/*main_utils.c */
int				init_all_coders(t_simulation *sim);
int				init_all_dongles(t_simulation *sim);
void			cleanup(t_simulation *sim, size_t dongles_done, size_t coders_done);
size_t			create_coders(t_simulation *sim, pthread_t *threads);
void			join_all(pthread_t *threads, size_t info[2], pthread_t monitor);

typedef struct s_heap_node
{
	t_coder	*coder;
	size_t	deadline;
}	t_heap_node;

typedef struct s_heap
{
	t_heap_node	*nodes;
	size_t		size;
	size_t		capacity;
}	t_heap;

typedef struct s_dongle
{
	int				id;
	int				is_taken;
	int				ever_used;
	size_t			timestamp;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	t_heap			waiting_heap;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	t_state			coder_status;
	size_t			nb_compiles;
	size_t			last_compile_start;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	t_simulation	*sim;
	pthread_t		coder_thread;
	pthread_mutex_t	compile_mutex;
}	t_coder;

typedef struct s_simulation
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
	int				flag;
	pthread_mutex_t	flag_mutex;
}	t_simulation;

#endif