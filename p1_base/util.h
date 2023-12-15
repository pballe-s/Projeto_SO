#ifndef UTIL_H
#define UTIL_H
	#include <stdlib.h>
	#include <unistd.h>
	#include <pthread.h>

	typedef struct arg{
		int thread_id, fd_in, fd_out, num_threads, leituras;
		pthread_mutex_t *file_lock;
	} arg;

	char	*ft_itoa(int n);
	int 	any(int *array, int search, int length);
	void 	next_line(int fd);
#endif