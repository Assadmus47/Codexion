/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 02:14:52 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/11 02:15:06 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_coder(t_simulation *sim, size_t i)
{
	size_t	elapsed;
	int		not_done;

	pthread_mutex_lock(&sim->coders[i].compile_mutex);
	elapsed = get_timestamp_ms() - sim->coders[i].last_compile_start;
	not_done = sim->coders[i].nb_compiles < sim->number_of_compiles_required;
	pthread_mutex_unlock(&sim->coders[i].compile_mutex);
	if (elapsed >= sim->time_to_burnout)
	{
		log_message(sim, sim->coders[i].id, "burned out");
		pthread_mutex_lock(&sim->flag_mutex);
		sim->flag = 1;
		pthread_mutex_unlock(&sim->flag_mutex);
		return (2);
	}
	return (not_done);
}

static int	check_all_coders(t_simulation *sim)
{
	size_t	i;
	int		status;
	int		all_done;

	i = 0;
	all_done = 1;
	while (i < sim->number_of_coders)
	{
		status = check_coder(sim, i);
		if (status == 2)
			return (1);
		if (status)
			all_done = 0;
		i++;
	}
	if (all_done)
	{
		pthread_mutex_lock(&sim->flag_mutex);
		sim->flag = 1;
		pthread_mutex_unlock(&sim->flag_mutex);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_simulation	*sim;
	int				stop;

	sim = (t_simulation *)arg;
	stop = 0;
	while (!stop)
	{
		if (check_all_coders(sim))
			return (NULL);
		pthread_mutex_lock(&sim->flag_mutex);
		stop = sim->flag;
		pthread_mutex_unlock(&sim->flag_mutex);
		usleep_ms(5);
	}
	return (NULL);
}
