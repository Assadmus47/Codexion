/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:56:41 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/12 02:06:53 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <sys/time.h>

static pthread_once_t	init_flag = PTHREAD_ONCE_INIT;
static struct timeval	start_tv;

static void	set_start_time(void)
{
	gettimeofday(&start_tv, NULL);
}

size_t	get_timestamp_ms(void)
{
	struct timeval	tv;
	long long		diff_usec;

	pthread_once(&init_flag, set_start_time);
	gettimeofday(&tv, NULL);
	diff_usec = (long long)(tv.tv_sec - start_tv.tv_sec) * 1000000
		+ (long long)(tv.tv_usec - start_tv.tv_usec);
	return ((size_t)(diff_usec / 1000));
}

void	usleep_ms(size_t ms)
{
	usleep(1000 * ms);
}

struct timespec	get_future_timespec(size_t ms)
{
	struct timeval	tv;
	struct timespec	ts;

	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec + (ms / 1000);
	ts.tv_nsec = (tv.tv_usec * 1000) + ((ms % 1000) * 1000000);
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000000;
	}
	return (ts);
}
