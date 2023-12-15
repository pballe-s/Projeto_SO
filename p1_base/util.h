#ifndef UTIL_H
#define UTIL_H
	#include <stdlib.h>
	#include <unistd.h>

	typedef struct arg{
		int thread_id, fd_in, fd_out, num_threads, leituras;
	} arg;

	char	*ft_itoa(int n);
	int 	any(int *array, int search, int length);
	void 	next_line(int fd);
#endif