#include "codexion.h"

int	main(int argc, char **argv)
{
	size_t	values[7];
	int		i;

	if (!parse_args(argc, argv, values))
		return (1);
	printf("timestamp = %zu\n", get_timestamp_ms());
	i = 0;
	while (i < 7)
	{
		printf("arg %d = %zu\n", i + 1, values[i]);
		i++;
	}
	printf("scheduler = %s\n", argv[8]);
	t_simulation	sim;

	if (pthread_mutex_init(&sim.log_mutex, NULL) != 0)
	{
		fprintf(stderr, "Error: mutex init failed\n");
		return (1);
	}
	log_message(&sim, 1, "is compiling");
	pthread_mutex_destroy(&sim.log_mutex);
	return (0);
}