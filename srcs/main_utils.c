/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 04:29:30 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/11 13:20:48 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_all_dongles(t_simulation *sim)
{
	size_t	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->dongles[i].id = (int)i;
		sim->dongles[i].is_taken = 0;
		sim->dongles[i].timestamp = 0;
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
		{
			cleanup(sim, i, 0);
			return (0);
		}
		if (!heap_init(&sim->dongles[i].waiting_heap, sim->number_of_coders))
		{
			pthread_mutex_destroy(&sim->dongles[i].mutex);
			cleanup(sim, i, 0);
			return (0);
		}
		if (pthread_cond_init(&sim->dongles[i].cond, NULL) != 0)
		{
			pthread_mutex_destroy(&sim->dongles[i].mutex);
			heap_destroy(&sim->dongles[i].waiting_heap);
			cleanup(sim, i, 0);
			return (0);
		}
		i++;
	}
	return (1);
}

int	init_all_coders(t_simulation *sim)
{
	size_t	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].id = (int)i + 1;
		sim->coders[i].sim = sim;
		sim->coders[i].nb_compiles = 0;
		sim->coders[i].coder_status = REFACTORING;
		sim->coders[i].left_dongle = &sim->dongles[i];
		if (sim->number_of_coders == 1)
			sim->coders[i].right_dongle = &sim->dongles[i];
		else
			sim->coders[i].right_dongle = &sim->dongles[(i + 1) % sim->number_of_coders];
		if (pthread_mutex_init(&sim->coders[i].compile_mutex, NULL) != 0)
		{
			cleanup(sim, sim->number_of_coders, i);
			return (0);
		}
		sim->coders[i].last_compile_start = get_timestamp_ms();
		i++;
	}
	return (1);
}

size_t	create_coders(t_simulation *sim, pthread_t *threads)
{
	size_t	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (pthread_create(&threads[i], NULL, coder_routine,
				&sim->coders[i]) != 0)
			break ;
		i++;
	}
	return (i);
}

void	join_all(pthread_t *threads, size_t info[2], pthread_t monitor)
{
	size_t	i;

	i = 0;
	while (i < info[0])
	{
		pthread_join(threads[i], NULL);
		i++;
	}
	if (info[1])
		pthread_join(monitor, NULL);
}
