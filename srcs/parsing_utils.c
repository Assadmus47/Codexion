/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:56:35 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/10 06:29:33 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_valid_number(const char *str)
{
	int	i;

	i = 0;
	if (str[0] == '\0')
		return 0;

	while (str[i])
	{
		if (!is_digit(str[i]))
			return 0;
		i++;
	}
	return (1);
}

size_t	ft_str_to_size(const char *str, int *valid)
{
	size_t	result;
	int		i;
	int		digit;

	result = 0;
	i = 0;
	*valid = 1;
	while (str[i])
	{
		digit = str[i] - '0';
		if (result > (SIZE_MAX - (size_t)digit) / 10)
		{
			*valid = 0;
			return (0);
		}
		result = result * 10 + (size_t)digit;
		i++;
	}
	return (result);
}

int	is_valid_scheduler(const char *str)
{
	if (strcmp(str, "fifo") == 0)
		return (1);
	if (strcmp(str, "edf") == 0)
		return (1);
	return (0);
}
