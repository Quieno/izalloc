/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_xxxxxx.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apalese- <apalese-@student.42malaga.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 202X/XX/XX XX:XX:XX by apalese-          #+#    #+#             */
/*   Updated: 202X/XX/XX XX:XX:XX by apalese-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IZALLOC_H
# define IZALLOC_H

typedef unsigned long long	t_ull;
typedef unsigned char		t_uc;

/* Settings */
# define MAX_MEMORY 1048576
# define MAX_PARTITIONS 1024

/* Callable functions */
void	*ft_alloc(t_ull size);
void	*ft_free(void *pointer);
void	*ft_realloc(void *pointer, t_ull size);

/* Internal functions */
void	*not_alloc(t_ull size, t_uc *mem, t_ull *atlas, t_ull *count);
void	*not_free(t_uc *p, t_uc *mem, t_ull *atlas, t_ull *count);
void	*not_realloc(t_uc *pack, t_uc *mem, t_ull *atlas, t_ull *count);

#endif
