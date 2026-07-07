#include "codexion.h"

int	parse_args(int argc, char **argv, size_t *values)
{
	int	valid;
	int	i;

	if (argc != 9)
	{
		fprintf(stderr, "Error: wrong number of arguments\n");
		return (0);
	}
	i = 1;
	while (i <= 7)
	{
		if (!is_valid_number(argv[i]))
		{
			fprintf(stderr, "Error: invalid argument\n");
			return (0);
		}
		values[i - 1] = ft_str_to_size(argv[i], &valid);
		if (!valid)
		{
			fprintf(stderr, "Error: overflow on argument\n");
			return (0);
		}
		i++;
	}
	if (values[0] == 0 || values[1] == 0 || values[5] == 0)
	{
		fprintf(stderr, "Error: coders, burnout time and required compiles must be > 0\n");
		return (0);
	}
	if (!is_valid_scheduler(argv[8]))
	{
		fprintf(stderr, "Error: scheduler must be fifo or edf\n");
		return (0);
	}
	return (1);
}
