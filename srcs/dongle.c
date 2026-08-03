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
	dongle->timestamp = get_timestamp_ms();
	pthread_mutex_unlock(&dongle->mutex);
}
