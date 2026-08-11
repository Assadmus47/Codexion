/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logging.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:56:28 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/11 13:27:41 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_message(t_simulation *sim, int coder_id, char *message)
{
	size_t	time;
	int		flag;

	pthread_mutex_lock(&sim->log_mutex);
	pthread_mutex_lock(&sim->flag_mutex);
	flag = sim->flag;
	pthread_mutex_unlock(&sim->flag_mutex);
	if (!flag || strcmp(message, "burned out") == 0)
	{
		time = get_timestamp_ms();
		printf("%zu %d %s \n", time, coder_id, message);
	}
	pthread_mutex_unlock(&sim->log_mutex);
}
