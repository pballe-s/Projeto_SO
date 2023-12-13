#ifndef UTIL_H
#define UTIL_H
	#include <stdlib.h>

	typedef struct arg{
		int thread_id, fd_in, fd_out;
	} arg;

	char	*ft_itoa(int n);
#endif