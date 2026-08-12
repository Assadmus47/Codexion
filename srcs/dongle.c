/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:56:21 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/12 03:55:47 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	use_dongle(t_dongle *dongle, size_t cooldown)
{
	size_t	time;

	pthread_mutex_lock(&dongle->mutex);
	if (dongle->is_taken == 1)
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (0);
	}
	time = get_timestamp_ms();
	if (cooldown > time - dongle->timestamp)
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (0);
	}
	dongle->is_taken = 1;
	pthread_mutex_unlock(&dongle->mutex);
	return (1);
}

void	release_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->is_taken = 0;
	dongle->ever_used = 1;
	dongle->timestamp = get_timestamp_ms();
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongles(t_coder *coder)
{
	release_dongle(coder->left_dongle);
	release_dongle(coder->right_dongle);
}

int	acquire_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	if (!acquire_one_dongle(coder, first))
		return (0);
	log_message(coder->sim, coder->id, "has taken a dongle");
	if (!acquire_one_dongle(coder, second))
	{
		release_dongle(first);
		return (0);
	}
	log_message(coder->sim, coder->id, "has taken a dongle");
	return (1);
}

