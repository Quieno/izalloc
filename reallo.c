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

static void	ft_fetch(t_uc *dest, t_uc *src, t_ull n)
{
	t_ull	i;

	i = 0;
	if (src < dest)
	{
		while (n-- > 0)
			dest[n] = src[n];
	}
	else
	{
		while (i < n)
		{
			dest[i] = src[i];
			i++;
		}
	}
}

static t_ull	ft_search(t_ull index, t_ull *atlas, t_ull counter)
{
	t_ull	start;
	t_ull	end;
	t_ull	center;

	start = 0;
	end = counter - 1;
	while (start <= end)
	{
		center = (start + end) / 2;
		if (atlas[center * 2] <= index && atlas[center * 2 + 1] >= index)
			return (center * 2);
		else if (atlas[center * 2] > index)
			end = center - 1;
		else
			start = center + 1;
	}
	return (MAX_PARTITIONS * 2);
}

static void	*ft_stay(t_ull index, t_ull size, t_uc *p, t_ull *atlas)
{
	t_ull	next_start;

	if (atlas[index + 2] != 0)
		next_start = atlas[index + 2];
	else
		next_start = MAX_MEMORY;
	if (size <= next_start - atlas[index])
	{
		atlas[index + 1] = atlas[index] + size - 1;
		return (p);
	}
	return ((void *)0);
}

void	*not_realloc(t_uc *packed, t_uc *mem, t_ull *atlas, t_ull *count)
{
	t_uc	*p;
	t_ull	size;
	t_ull	index;
	t_uc	*new_ptr;

	p = (t_uc *)((t_ull *)packed)[0];
	size = ((t_ull *)packed)[1];
	index = ft_search((t_ull)(p - mem), atlas, *count);
	if (index == MAX_PARTITIONS * 2)
		return ((void *)0);
	if (ft_stay(index, size, p, atlas) != (void *)0)
		return (p);
	index = atlas[index + 1] - atlas[index] + 1;
	not_free(p, mem, atlas, count);
	new_ptr = (t_uc *)not_alloc(size, mem, atlas, count);
	if (new_ptr != (void *)0 && new_ptr != p)
	{
		if (index < size)
			ft_fetch(new_ptr, p, index);
		else
			ft_fetch(new_ptr, p, size);
	}
	return (new_ptr);
}
