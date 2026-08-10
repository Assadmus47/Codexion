/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mkacemi <mkacemi@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 04:56:25 by mkacemi           #+#    #+#             */
/*   Updated: 2026/08/09 04:56:26 by mkacemi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_init(t_heap *heap, size_t capacity)
{
	heap->nodes = malloc(sizeof(t_heap_node) * capacity);
	if (!heap->nodes)
		return (0);
	heap->size = 0;
	heap->capacity = capacity;
	return (1);
}

void	heap_push(t_heap *heap, t_coder *coder, size_t priority)
{
	size_t	i;
	size_t	parent;
	t_heap_node	tmp;

	heap->nodes[heap->size].coder = coder;
	heap->nodes[heap->size].deadline = priority;
	i = heap->size;
	heap->size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (heap->nodes[parent].deadline <= heap->nodes[i].deadline)
			break ;
		tmp = heap->nodes[parent];
		heap->nodes[parent] = heap->nodes[i];
		heap->nodes[i] = tmp;
		i = parent;
	}
}

t_coder	*heap_pop(t_heap *heap)
{
	t_coder		*result;
	size_t		i;
	size_t		left;
	size_t		right;
	size_t		smallest;
	t_heap_node	tmp;

	result = heap->nodes[0].coder;
	heap->size--;
	heap->nodes[0] = heap->nodes[heap->size];
	i = 0;
	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < heap->size && (heap->nodes[left].deadline < heap->nodes[smallest].deadline
			|| (heap->nodes[left].deadline == heap->nodes[smallest].deadline
			&& heap->nodes[left].coder->id < heap->nodes[smallest].coder->id)))
			smallest = left;
		if (right < heap->size && (heap->nodes[right].deadline < heap->nodes[smallest].deadline
			|| (heap->nodes[right].deadline == heap->nodes[smallest].deadline
			&& heap->nodes[right].coder->id < heap->nodes[smallest].coder->id)))
			smallest = right;
		if (smallest == i)
			break ;
		tmp = heap->nodes[i];
		heap->nodes[i] = heap->nodes[smallest];
		heap->nodes[smallest] = tmp;
		i = smallest;
	}
	return (result);
}

void	heap_destroy(t_heap *heap)
{
	free(heap->nodes);
	heap->size = 0;
	heap->capacity = 0;
}

size_t	get_priority(t_coder *coder)
{
	if (coder->sim->scheduler == FIFO)
		return (get_timestamp_ms());
	return (coder->last_compile_start + coder->sim->time_to_burnout);
}
