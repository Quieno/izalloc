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

static t_ull	ft_seek(t_ull index, t_ull *atlas, t_ull counter)
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
	return (1);
}

static void	ft_destroy(t_ull index, t_ull *atlas, t_ull *counter)
{
	t_ull	i;

	i = index;
	while (i < (*counter - 1) * 2)
	{
		atlas[i] = atlas[i + 2];
		atlas[i + 1] = atlas[i + 3];
		i = i + 2;
	}
	atlas[(*counter - 1) * 2] = 0;
	atlas[(*counter - 1) * 2 + 1] = 0;
	(*counter)--;
}

void	*not_free(t_uc *p, t_uc *mem, t_ull *atlas, t_ull *counter)
{
	t_ull	index;

	if ((t_ull)p >= (t_ull)mem && (t_ull)p < (t_ull) & mem[MAX_MEMORY])
	{
		if (*counter != 0)
		{
			index = ft_seek((t_ull)(p - mem), atlas, *counter);
			if (index != 1)
				ft_destroy(index, atlas, counter);
		}
	}
	return ((void *)0);
}
