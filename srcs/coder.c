/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:56:12 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/12 04:34:18 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	get_simulation_flag(t_coder *coder)
{
	int	stop;

	pthread_mutex_lock(&coder->sim->flag_mutex);
	stop = coder->sim->flag;
	pthread_mutex_unlock(&coder->sim->flag_mutex);
	return (stop);
}

static int	run_one_cycle(t_coder *coder)
{
	if (!acquire_dongles(coder))
		return (1);
	if (get_simulation_flag(coder))
	{
		release_dongles(coder);
		return (1);
	}
	coder->coder_status = COMPILING;
	log_message(coder->sim, coder->id, "is compiling");
	pthread_mutex_lock(&coder->compile_mutex);
	coder->last_compile_start = get_timestamp_ms();
	pthread_mutex_unlock(&coder->compile_mutex);
	interruptible_sleep(coder, coder->sim->time_to_compile);
	pthread_mutex_lock(&coder->compile_mutex);
	coder->nb_compiles++;
	pthread_mutex_unlock(&coder->compile_mutex);
	release_dongles(coder);
	if (get_simulation_flag(coder))
		return (1);
	coder->coder_status = DEBUGGING;
	log_message(coder->sim, coder->id, "is debugging");
	interruptible_sleep(coder, coder->sim->time_to_debug);
	if (get_simulation_flag(coder))
		return (1);
	coder->coder_status = REFACTORING;
	log_message(coder->sim, coder->id, "is refactoring");
	interruptible_sleep(coder, coder->sim->time_to_refactor);
	return (get_simulation_flag(coder));
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	usleep_ms((size_t)coder->id % 10);
	while (!get_simulation_flag(coder))
	{
		if (run_one_cycle(coder))
			break ;
	}
	return (NULL);
}
