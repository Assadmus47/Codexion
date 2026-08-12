/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_wait.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:56:18 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/12 03:56:35 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_dongle_ready(t_coder *coder, t_dongle *dongle)
{
	size_t	now;

	if (dongle->is_taken)
		return (0);
	if (dongle->waiting_heap.size > 0
		&& dongle->waiting_heap.nodes[0].coder->id != coder->id)
		return (0);
	if (!dongle->ever_used)
		return (1);
	now = get_timestamp_ms();
	if (now - dongle->timestamp < coder->sim->dongle_cooldown)
		return (0);
	return (1);
}

static int	wait_for_dongle(t_coder *coder, t_dongle *dongle, size_t priority)
{
	struct timespec	ts;

	heap_push(&dongle->waiting_heap, coder, priority);
	while (!is_dongle_ready(coder, dongle))
	{
		ts = get_future_timespec(2);
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		if (get_simulation_flag(coder))
			return (0);
	}
	heap_pop(&dongle->waiting_heap);
	return (1);
}

int	acquire_one_dongle(t_coder *coder, t_dongle *dongle)
{
	size_t	priority;
	int		ok;

	priority = get_priority(coder);
	pthread_mutex_lock(&dongle->mutex);
	ok = wait_for_dongle(coder, dongle, priority);
	if (ok)
		dongle->is_taken = 1;
	pthread_mutex_unlock(&dongle->mutex);
	return (ok);
}
