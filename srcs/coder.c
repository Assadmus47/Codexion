/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:56:12 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/09 04:56:15 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int get_simulation_flag(t_coder *coder)
{
	int stop;

	pthread_mutex_lock(&coder->sim->flag_mutex);
	stop = coder->sim->flag;
	pthread_mutex_unlock(&coder->sim->flag_mutex);
	return (stop);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		stop;

	coder = (t_coder *)arg;
	stop = get_simulation_flag(coder);
	while (stop == 0)
	{
		if (!acquire_dongles(coder))
			break ;
		coder->coder_status = COMPILING;
		log_message(coder->sim, coder->id, "is compiling");
		pthread_mutex_lock(&coder->compile_mutex);
		coder->last_compile_start = get_timestamp_ms();
		pthread_mutex_unlock(&coder->compile_mutex);
		usleep_ms(coder->sim->time_to_compile);
		coder->nb_compiles++;
		release_dongles(coder);
		stop = get_simulation_flag(coder);
		if (stop)
			break ;
		coder->coder_status = DEBUGGING;
		log_message(coder->sim, coder->id, "is debugging");
		usleep_ms(coder->sim->time_to_debug);
		stop = get_simulation_flag(coder);
		if (stop)
			break ;
		coder->coder_status = REFACTORING;
		log_message(coder->sim, coder->id, "is refactoring");
		usleep_ms(coder->sim->time_to_refactor);
		stop = get_simulation_flag(coder);
	}
	return (NULL);
}

void	*monitor_routine(void *arg)
{
	t_simulation	*sim;
	size_t			i;
	size_t			elapsed;
	int				stop;
	int				all_done;

	sim = (t_simulation *)arg;
	stop = 0;
	while (!stop)
	{
		i = 0;
		all_done = 1;
		while (i < sim->number_of_coders)
		{
			pthread_mutex_lock(&sim->coders[i].compile_mutex);
			elapsed = get_timestamp_ms() - sim->coders[i].last_compile_start;
			pthread_mutex_unlock(&sim->coders[i].compile_mutex);
			if (elapsed >= sim->time_to_burnout)
			{
				log_message(sim, sim->coders[i].id, "burned out");
				pthread_mutex_lock(&sim->flag_mutex);
				sim->flag = 1;
				pthread_mutex_unlock(&sim->flag_mutex);
				return (NULL);
			}
			if (sim->coders[i].nb_compiles < sim->number_of_compiles_required)
				all_done = 0;
			i++;
		}
		if (all_done)
		{
			pthread_mutex_lock(&sim->flag_mutex);
			sim->flag = 1;
			pthread_mutex_unlock(&sim->flag_mutex);
			return (NULL);
		}
		pthread_mutex_lock(&sim->flag_mutex);
		stop = sim->flag;
		pthread_mutex_unlock(&sim->flag_mutex);
		usleep_ms(5);
	}
	return (NULL);
}
