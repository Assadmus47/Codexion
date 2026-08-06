
#include "codexion.h"

int	main(void)
{
	t_simulation	sim;
	t_coder			coders[3];
	t_dongle		dongles[3];
	pthread_t		threads[3];
	pthread_t		monitor_thread;
	int				j;

	sim.time_to_compile = 100;
	sim.time_to_debug = 100;
	sim.time_to_refactor = 100;
	sim.dongle_cooldown = 0;
	sim.time_to_burnout = 150;
	sim.number_of_coders = 3;
	sim.coders = coders;
	pthread_mutex_init(&sim.log_mutex, NULL);
	pthread_mutex_init(&sim.flag_mutex, NULL);
	sim.flag = 0;
	j = 0;
	while (j < 3)
	{
		dongles[j].id = j;
		dongles[j].is_taken = 0;
		dongles[j].timestamp = 0;
		pthread_mutex_init(&dongles[j].mutex, NULL);
		j++;
	}
	j = 0;
	while (j < 3)
	{
		coders[j].id = j + 1;
		coders[j].sim = &sim;
		coders[j].nb_compiles = 0;
		coders[j].left_dongle = &dongles[j];
		coders[j].right_dongle = &dongles[(j + 1) % 3];
		pthread_mutex_init(&coders[j].compile_mutex, NULL);
		coders[j].last_compile_start = get_timestamp_ms();
		pthread_create(&threads[j], NULL, coder_routine, &coders[j]);
		j++;
	}
	pthread_create(&monitor_thread, NULL, monitor_routine, &sim);
	sleep(2);
	pthread_mutex_lock(&sim.flag_mutex);
	sim.flag = 1;
	pthread_mutex_unlock(&sim.flag_mutex);
	j = 0;
	while (j < 3)
	{
		pthread_join(threads[j], NULL);
		j++;
	}
	pthread_join(monitor_thread, NULL);
	pthread_mutex_destroy(&sim.log_mutex);
	pthread_mutex_destroy(&sim.flag_mutex);
	return (0);
}