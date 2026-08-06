#include "codexion.h"

// int	main(int argc, char **argv)
// {
// 	size_t	values[7];
// 	int		i;

// 	if (!parse_args(argc, argv, values))
// 		return (1);
// 	printf("timestamp = %zu\n", get_timestamp_ms());
// 	i = 0;
// 	while (i < 7)
// 	{
// 		printf("arg %d = %zu\n", i + 1, values[i]);
// 		i++;
// 	}
// 	printf("scheduler = %s\n", argv[8]);
// 	t_simulation	sim;

// 	if (pthread_mutex_init(&sim.log_mutex, NULL) != 0)
// 	{
// 		fprintf(stderr, "Error: mutex init failed\n");
// 		return (1);
// 	}
// 	log_message(&sim, 1, "is compiling");
// 	pthread_mutex_destroy(&sim.log_mutex);

// 	t_coder		coders[3];
// 	pthread_t	threads[3];
// 	int			j;
// 	j = 0;
// 	while (j < 3)
// 	{
// 		coders[j].id = j;
// 		pthread_create(&threads[j], NULL, coder_routine, &coders[j]);
// 		j++;
// 	}
// 	j = 0;
// 	while (j < 3)
// 	{
// 		pthread_join(threads[j], NULL);
// 		j++;
// 	}
// 	return (0);
// }
// int	main(void)
// {
// 	t_heap	heap;
// 	t_coder	c1;
// 	t_coder	c2;
// 	t_coder	c3;
// 	t_coder	*result;

// 	c1.id = 1;
// 	c2.id = 2;
// 	c3.id = 3;
// 	heap_init(&heap, 3);
// 	heap_push(&heap, &c1, 500);
// 	heap_push(&heap, &c2, 100);
// 	heap_push(&heap, &c3, 300);
// 	result = heap_pop(&heap);
// 	printf("pop 1: coder %d\n", result->id);
// 	result = heap_pop(&heap);
// 	printf("pop 2: coder %d\n", result->id);
// 	result = heap_pop(&heap);
// 	printf("pop 3: coder %d\n", result->id);
// 	heap_destroy(&heap);
// }
int	main(void)
{
	t_simulation	sim;
	t_coder			coders[3];
	t_dongle		dongles[3];
	pthread_t		threads[3];
	int				j;

	sim.time_to_compile = 100;
	sim.time_to_debug = 100;
	sim.time_to_refactor = 100;
	sim.dongle_cooldown = 0;
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
		pthread_create(&threads[j], NULL, coder_routine, &coders[j]);
		j++;
	}
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
	pthread_mutex_destroy(&sim.log_mutex);
	pthread_mutex_destroy(&sim.flag_mutex);
	return (0);
}
