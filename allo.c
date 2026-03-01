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

static void	ft_record(t_ull *bar, t_ull start, t_ull size)
{
	bar[0] = start;
	bar[1] = size;
}

static void	ft_extremes(t_ull *bar, t_ull size, t_ull *atlas, t_ull counter)
{
	t_ull	gap;

	if (counter == 0)
	{
		if (size <= MAX_MEMORY)
			return (ft_record(bar, 0, size));
		return ;
	}
	if (atlas[0] == size && size <= MAX_MEMORY)
		return (ft_record(bar, 0, size));
	if (atlas[0] > size && (bar[1] == 0 || atlas[0] < bar[1]))
		ft_record(bar, 0, atlas[0]);
	gap = atlas[2 * counter - 1] + 1;
	if (gap % 16 != 0)
		gap = gap + 16 - (gap % 16);
	gap = MAX_MEMORY - gap;
	if (gap >= size && (bar[1] == 0 || gap < bar[1]))
		ft_record(bar, MAX_MEMORY - gap, gap);
}

static void	ft_middle(t_ull *bar, t_ull size, t_ull *atlas, t_ull counter)
{
	t_ull	i;
	t_ull	gap;

	i = 1;
	while (bar[1] != size && i <= (counter * 2) - 1)
	{
		gap = atlas[i] + 1;
		if (gap % 16 != 0)
			gap = gap + 16 - (gap % 16);
		gap = atlas[i + 1] - gap;
		if (gap >= size && (bar[1] == 0 || gap < bar[1]))
			ft_record(bar, atlas[i + 1] - gap, gap);
		i = i + 2;
	}
}

static void	ft_scribe(t_ull start, t_ull end, t_ull *atlas, t_ull *count)
{
	t_ull	i;
	t_ull	j;

	i = 0;
	j = *count * 2;
	while (i < j && atlas[i] < start)
		i = i + 2;
	while (j > i)
	{
		atlas[j + 1] = atlas[j - 1];
		atlas[j] = atlas[j - 2];
		j = j - 2;
	}
	atlas[i] = start;
	atlas[i + 1] = start + end - 1;
	(*count)++;
}

void	*not_alloc(t_ull size, t_uc *mem, t_ull *atlas, t_ull *count)
{
	t_ull	bar[2];

	bar[1] = 0;
	ft_extremes(bar, size, atlas, *count);
	if (*count >= 2)
		ft_middle(bar, size, atlas, *count);
	if (bar[1] != 0)
	{
		ft_scribe(bar[0], size, atlas, count);
		return (&mem[bar[0]]);
	}
	return ((void *)0);
}
