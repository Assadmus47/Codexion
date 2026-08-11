/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:56:32 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/12 01:28:02 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup(t_simulation *sim, size_t dongles_done, size_t coders_done)
{
	size_t	i;

	i = 0;
	while (i < dongles_done)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		heap_destroy(&sim->dongles[i].waiting_heap);
		pthread_cond_destroy(&sim->dongles[i].cond);
		i++;
	}
	i = 0;
	while (i < coders_done)
	{
		pthread_mutex_destroy(&sim->coders[i].compile_mutex);
		i++;
	}
	if (sim->dongles)
		free(sim->dongles);
	if (sim->coders)
		free(sim->coders);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->flag_mutex);
}

static int	init_simulation(t_simulation *sim, size_t *values,
		char *scheduler_str)
{
	sim->number_of_coders = values[0];
	sim->time_to_burnout = values[1];
	sim->time_to_compile = values[2];
	sim->time_to_debug = values[3];
	sim->time_to_refactor = values[4];
	sim->number_of_compiles_required = values[5];
	sim->dongle_cooldown = values[6];
	if (strcmp(scheduler_str, "fifo") == 0)
		sim->scheduler = FIFO;
	else
		sim->scheduler = EDF;
	sim->flag = 0;
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->flag_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->log_mutex);
		return (0);
	}
	return (1);
	return (1);
}

static int	init_arrays(t_simulation *sim)
{
	sim->dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	if (!sim->dongles)
		return (0);
	sim->coders = malloc(sizeof(t_coder) * sim->number_of_coders);;
	if (!sim->coders)
	{
		free(sim->dongles);
		return (0);
	}
	if (!init_all_dongles(sim))
		return (0);
	if (!init_all_coders(sim))
		return (0);
	return (1);
}

static void	run_simulation(t_simulation *sim)
{
	pthread_t	*threads;
	pthread_t	monitor_thread;
	size_t		info[2];

	threads = malloc(sizeof(pthread_t) * sim->number_of_coders);
	if (!threads)
	{
		cleanup(sim, sim->number_of_coders, sim->number_of_coders);
		return ;
	}
	info[0] = create_coders(sim, threads);
	info[1] = 0;
	monitor_thread = 0;
	if (info[0] == sim->number_of_coders)
	{
		if (pthread_create(&monitor_thread, NULL, monitor_routine, sim) == 0)
			info[1] = 1;
	}
	else
	{
		pthread_mutex_lock(&sim->flag_mutex);
		sim->flag = 1;
		pthread_mutex_unlock(&sim->flag_mutex);
	}
	join_all(threads, info, monitor_thread);
	free(threads);
	cleanup(sim, sim->number_of_coders, sim->number_of_coders);
}

int	main(int argc, char **argv)
{
	size_t			values[7];
	t_simulation	sim;

	if (!parse_args(argc, argv, values))
		return (1);
	if (!init_simulation(&sim, values, argv[8]))
		return (1);
	if (!init_arrays(&sim))
		return (1);
	run_simulation(&sim);
	return (0);
}
