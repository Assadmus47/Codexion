#include "codexion.h"
#include <sys/time.h>


size_t	get_timestamp_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((size_t)(tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	usleep_ms(size_t ms)
{
	usleep(1000 * ms);
}