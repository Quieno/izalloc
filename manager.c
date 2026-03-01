/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_xxxxxx.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apalese- <apalese-@student.42malaga.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 202X/XX/XX XX:XX:XX by apalese-          #+#    #+#             */
/*   Updated: 202X/XX/XX XX:XX:XX by apalese-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "izalloc.h"

/* This is the central part of this project, i don't know how to use structures
 yet so it was limited.
memory will store the data itself, atlas is mapping the used regions, and count
er simplifies the logic down the road by keeping track of how many live blocks 
exist at any given moment */
static void	*ft_manager(char mode, t_ull size, t_uc *pointer)
{
	static t_uc		memory[MAX_MEMORY];
	static t_ull	atlas[MAX_PARTITIONS * 2];
	static t_ull	counter;

	if (mode == 0 && counter < MAX_PARTITIONS)
		return (not_alloc(size, memory, atlas, &counter));
	else if (mode == 1 && counter > 0)
		return (not_free(pointer, memory, atlas, &counter));
	else if (mode == 2 && counter > 0)
		return (not_realloc(pointer, memory, atlas, &counter));
	return ((void *)0);
}

/* This are the functions you will call, they mostly share the same behaviour
as the standard ones */
void	*ft_alloc(t_ull size)
{
	if (size > 0)
		return (ft_manager(0, size, (void *)0));
	return ((void *)0);
}

/* two important changes were made to free, they dont break any standard use.
ft_free always returns a NULL pointer, it also can free a block with a pointer
to any of its parts, not just the first position */
void	*ft_free(void *pointer)
{
	if (pointer != (void *)0)
		ft_manager(1, 0, (t_uc *)pointer);
	return ((void *)0);
}

void	*ft_realloc(void *pointer, t_ull size)
{
	t_ull	pack[2];

	if (size == 0)
		return (ft_free(pointer));
	if (pointer == (void *)0)
		return (ft_alloc(size));
	pack[0] = (t_ull)pointer;
	pack[1] = size;
	return (ft_manager(2, 0, (t_uc *)pack));
}
