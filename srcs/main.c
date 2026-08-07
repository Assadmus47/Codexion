#include "codexion.h"

static int	init_simulation(t_simulation *sim, size_t *values, char *scheduler_str)
{
	sim->number_of_coders = values[0];
	sim->time_to_burnout = values[1];
	sim->time_to_compile = values[2];
	sim->time_to_debug = values[3];
	sim->time_to_refactor = values[4];
	sim->number_of_compiles_required = values[5];
	sim->dongle_cooldown = values[6];
	if (strcmp(scheduler_str, "fifo") == 0)
		sim->scheduler = FIFO;
	else
		sim->scheduler = EDF;
	sim->flag = 0;
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->flag_mutex, NULL) != 0)
		return (0);
	return (1);
}

static int	init_arrays(t_simulation *sim)
{
	size_t	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	sim->coders = malloc(sizeof(t_coder) * sim->number_of_coders);
	if (!sim->dongles || !sim->coders)
		return (0);
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->dongles[i].id = (int)i;
		sim->dongles[i].is_taken = 0;
		sim->dongles[i].timestamp = 0;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		if (!heap_init(&sim->dongles[i].waiting_heap, sim->number_of_coders))
			return (0);
		pthread_cond_init(&sim->dongles[i].cond, NULL);
		i++;
	}
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].id = (int)i + 1;
		sim->coders[i].sim = sim;
		sim->coders[i].nb_compiles = 0;
		sim->coders[i].coder_status = REFACTORING;
		sim->coders[i].left_dongle = &sim->dongles[i];
		if (sim->number_of_coders == 1)
			sim->coders[i].right_dongle = &sim->dongles[i];
		else
			sim->coders[i].right_dongle = &sim->dongles[(i + 1) % sim->number_of_coders];
		pthread_mutex_init(&sim->coders[i].compile_mutex, NULL);
		sim->coders[i].last_compile_start = get_timestamp_ms();
		i++;
	}
	return (1);
}

static void	cleanup(t_simulation *sim)
{
	size_t	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		heap_destroy(&sim->dongles[i].waiting_heap);
		pthread_cond_destroy(&sim->dongles[i].cond);
		pthread_mutex_destroy(&sim->coders[i].compile_mutex);
		i++;
	}
	free(sim->dongles);
	free(sim->coders);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->flag_mutex);
}

int	main(int argc, char **argv)
{
	size_t			values[7];
	t_simulation	sim;
	pthread_t		*threads;
	pthread_t		monitor_thread;
	size_t			i;

	if (!parse_args(argc, argv, values))
		return (1);
	if (!init_simulation(&sim, values, argv[8]))
		return (1);
	if (!init_arrays(&sim))
		return (1);
	threads = malloc(sizeof(pthread_t) * sim.number_of_coders);
	if (!threads)
		return (1);
	i = 0;
	while (i < sim.number_of_coders)
	{
		pthread_create(&threads[i], NULL, coder_routine, &sim.coders[i]);
		i++;
	}
	pthread_create(&monitor_thread, NULL, monitor_routine, &sim);
	i = 0;
	while (i < sim.number_of_coders)
	{
		pthread_join(threads[i], NULL);
		i++;
	}
	pthread_join(monitor_thread, NULL);
	free(threads);
	cleanup(&sim);
	return (0);
}