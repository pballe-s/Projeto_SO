#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>

#include "constants.h"
#include "operations.h"
#include "parser.h"
#include "util.h"

void *process_file(void *args)
{
	int end_file = 0, *exit_value = malloc(sizeof(int));
	arg *buffer = (arg *)args;
	*exit_value = 0;
	
	while (1) {
    	unsigned int event_id, delay, wait_tid = 0;
    	size_t num_rows, num_columns, num_coords;
		size_t xs[MAX_RESERVATION_SIZE], ys[MAX_RESERVATION_SIZE];
		

		switch (get_next(buffer->fd_in)) {
  		case CMD_CREATE:
    		if (parse_create(buffer->fd_in, &event_id, &num_rows, &num_columns) != 0) {
          		fprintf(stderr, "Invalid command. See HELP for usage\n");	
          		continue;
        	}

    		if (ems_create(event_id, num_rows, num_columns)) {
      			fprintf(stderr, "Failed to create event\n");
    		}

    		break;

      	case CMD_RESERVE:
        	num_coords = parse_reserve(buffer->fd_in, MAX_RESERVATION_SIZE, &event_id, xs, ys);

        	if (num_coords == 0) {
          		fprintf(stderr, "Invalid command. See HELP for usage\n");
          		continue;
        	}

        	if (ems_reserve(event_id, num_coords, xs, ys)) {
          		fprintf(stderr, "Failed to reserve seats\n");
        	}

        	break;

      	case CMD_SHOW:
        	if (parse_show(buffer->fd_in, &event_id) != 0) {
          		fprintf(stderr, "Invalid command. See HELP for usage\n");
          		continue;
        	}

        	if (ems_show(event_id, buffer->fd_out)) {
          		fprintf(stderr, "Failed to show event\n");
        	}

        break;

      	case CMD_LIST_EVENTS:
        	if (ems_list_events(buffer->fd_out)) {
          		fprintf(stderr, "Failed to list events\n");
        	}

       	break;

      	case CMD_WAIT:
        	if (parse_wait(buffer->fd_in, &delay, &wait_tid) == -1) {  // thread_id is not implemented
          		fprintf(stderr, "Invalid command. See HELP for usage\n");
          		continue;
        	}

        	if (delay > 0 && (wait_tid == 0 || (unsigned int)(buffer->thread_id + 1) == wait_tid)) {
          		printf("Waiting...\n");
          		ems_wait(delay);
        		}

        	break;

      	case CMD_INVALID:
        	fprintf(stderr, "Invalid command. See HELP for usage\n");
        	break;

      	case CMD_HELP:
        	printf(
            	"Available commands:\n"
            	"  CREATE <event_id> <num_rows> <num_columns>\n"
        		"  RESERVE <event_id> [(<x1>,<y1>) (<x2>,<y2>) ...]\n"
        		"  SHOW <event_id>\n"
        		"  LIST\n"
            	"  WAIT <delay_ms> [thread_id]\n"  // thread_id is not implemented
            	"  BARRIER\n"                      // Not implemented
        		"  HELP\n");

        	break;

      	case CMD_BARRIER:
			*exit_value = BARRIER_EXIT;
			pthread_exit((void*)exit_value);
    	case CMD_EMPTY:
    		break;

    	case EOC:
			end_file = 1;
	    	break;
		
    	}
		if (end_file)
			break;
  	}
	close(buffer->fd_in);
	close(buffer->fd_out);
	pthread_exit((void*)exit_value);
}

int main(int argc, char *argv[]) {
  unsigned int state_access_delay_ms = STATE_ACCESS_DELAY_MS;
  DIR *dir;
  struct dirent *file;
  char *path_files;
  char *cpypath;
  int pid, proc_cont = 0, max_procs = 3, estado = 0, max_threads = 3;

  if (argc > 3) {
    char *endptr;
    unsigned long int delay = strtoul(argv[3], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_ms = (unsigned int)delay;
  }

  if (argc > 3)
  {
	max_threads = atoi(argv[3]);
  }

  if (argc > 2)
  {
	max_procs = atoi(argv[2]);
  }

  if (argc > 1)
  {
	dir = opendir(argv[1]);
	path_files = strcat(argv[1], "/");
	cpypath = (char *)malloc(strlen(path_files)*sizeof(char *));
  }

  if (ems_init(state_access_delay_ms)) {
    fprintf(stderr, "Failed to initialize EMS\n");
    return 1;
  }

	while ((file = readdir(dir)) != NULL)
	{
		int fd_in, fd_out;
		

		if (!strstr(file->d_name, ".jobs"))
				continue;
		if(proc_cont == max_procs)
		{
			wait(&estado);
			printf("status: %d\n", estado);
			proc_cont--;
		}
		pid = fork();
		if (pid > 0)
			proc_cont++;
		if (pid == 0)
		{
			arg *buffer[max_threads];
			pthread_t tid[max_threads];
			int exit_values[max_threads], barrier = 0, *aux_exit_value;

			
			for (int i = 0; i < max_threads; i++)
			{
				buffer[i] = malloc(sizeof(arg));
				strcpy(cpypath, path_files);
				fd_in = open(strcat(cpypath, file->d_name), O_RDONLY);
				strcpy(cpypath, path_files);
				fd_out = open(strcat(cpypath, strcat(strtok(file->d_name, "."), ".out")), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
				buffer[i]->fd_in = fd_in;
				buffer[i]->fd_out = fd_out;
				buffer[i]->thread_id = i;
				pthread_create(&tid[i], 0, process_file, buffer[i]);
			}
			for (int i = 0; i < max_threads; i++)
			{
				pthread_join(tid[i], (void**)&aux_exit_value);
				exit_values[i] = *aux_exit_value;
			}
			barrier = any(exit_values, BARRIER_EXIT, max_threads);
			while (barrier)
			{
				for (int i = 0; i < max_threads; i++)
				{
					pthread_create(&tid[i], 0, process_file, buffer[i]);
				}
				for (int i = 0; i < max_threads; i++)
				{
					pthread_join(tid[i], (void**)&aux_exit_value);
					exit_values[i] = *aux_exit_value;	
				}
				barrier = any(exit_values, BARRIER_EXIT, max_threads);
			}
			free(aux_exit_value);
			exit(EXIT_SUCCESS);
		}
	}
	while(proc_cont > 0)
		{
			wait(&estado);
			printf("status: %d\n", estado);
			proc_cont--;
		}
	free(cpypath);
	ems_terminate();
}
