#include "codexion.h"

static int get_simulation_flag(t_coder *coder)
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
		acquire_dongles(coder);
		coder->coder_status = COMPILING;
		log_message(coder->sim, coder->id, "is compiling");
		coder->last_compile_start = get_timestamp_ms();
		usleep_ms(coder->sim->time_to_compile);
		coder->nb_compiles++;
		release_dongles(coder);
		coder->coder_status = DEBUGGING;
		log_message(coder->sim, coder->id, "is debugging");
		usleep_ms(coder->sim->time_to_debug);
		coder->coder_status = REFACTORING;
		log_message(coder->sim, coder->id, "is refactoring");
		usleep_ms(coder->sim->time_to_refactor);
		stop = get_simulation_flag(coder);
	}
	return (NULL);
}