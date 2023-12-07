#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

#include "constants.h"
#include "operations.h"
#include "parser.h"

int main(int argc, char *argv[]) {
  unsigned int state_access_delay_ms = STATE_ACCESS_DELAY_MS;
  DIR *dir;
  struct dirent *file;
  char *path_files;
  char *cpypath;

  if (argc > 2) {
    char *endptr;
    unsigned long int delay = strtoul(argv[2], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_ms = (unsigned int)delay;
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
		int fd_in, fd_out, end_file = 0;
		
		strcpy(cpypath, path_files);
		if (!strstr(file->d_name, ".jobs"))
			continue;
		fd_in = open(strcat(cpypath, file->d_name), O_RDONLY);
		strcpy(cpypath, path_files);
		fd_out = open(strcat(cpypath, strcat(strtok(file->d_name, "."), ".out")), O_RDWR | O_CREAT, "rw");
		while (1) {
    	unsigned int event_id, delay;
    	size_t num_rows, num_columns, num_coords;
    	size_t xs[MAX_RESERVATION_SIZE], ys[MAX_RESERVATION_SIZE];

    	switch (get_next(fd_in)) {
      	case CMD_CREATE:
        	if (parse_create(fd_in, &event_id, &num_rows, &num_columns) != 0) {
          	fprintf(stderr, "Invalid command. See HELP for usage\n");
          	continue;
        	}

        	if (ems_create(event_id, num_rows, num_columns)) {
          		fprintf(stderr, "Failed to create event\n");
        	}

        	break;

      	case CMD_RESERVE:
        	num_coords = parse_reserve(fd_in, MAX_RESERVATION_SIZE, &event_id, xs, ys);

        	if (num_coords == 0) {
          		fprintf(stderr, "Invalid command. See HELP for usage\n");
          		continue;
        	}

        	if (ems_reserve(event_id, num_coords, xs, ys)) {
          		fprintf(stderr, "Failed to reserve seats\n");
        	}

        	break;

      	case CMD_SHOW:
        	if (parse_show(fd_in, &event_id) != 0) {
          		fprintf(stderr, "Invalid command. See HELP for usage\n");
          		continue;
        }

        	if (ems_show(event_id, fd_out)) {
          		fprintf(stderr, "Failed to show event\n");
        	}

        break;

      	case CMD_LIST_EVENTS:
        	if (ems_list_events(fd_out)) {
          		fprintf(stderr, "Failed to list events\n");
        	}

       	break;

      	case CMD_WAIT:
        	if (parse_wait(fd_in, &delay, NULL) == -1) {  // thread_id is not implemented
          		fprintf(stderr, "Invalid command. See HELP for usage\n");
          	continue;
        	}

        	if (delay > 0) {
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

      	case CMD_BARRIER:  // Not implemented
    	case CMD_EMPTY:
        	break;

    	case EOC:
			end_file = 1;
        break;
		
    	}
		if (end_file)
			break;
  		}
		close(fd_in);
		close(fd_out);
	}
	free(cpypath);
  	ems_terminate();
}
