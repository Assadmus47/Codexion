#include "codexion.h"

void	log_message(t_simulation *sim, int coder_id, char *message)
{
	size_t	time;

	pthread_mutex_lock(&sim->log_mutex);
	time = get_timestamp_ms();
	printf("%zu %d %s \n", time, coder_id, message);
	pthread_mutex_unlock(&sim->log_mutex);
}