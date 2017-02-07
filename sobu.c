#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

#include "sobu.h"

#define MAX_BUF 1024

ssize_t readln (int fildes, void* buf, size_t nbyte){
	int n;
	size_t i = 0;
	char* b = (char*) buf;

	while (i < nbyte - 2){
		n = read (fildes, &b[i], 1);

		if (n <= 0 || b[i] == 0 || b[i] == '\n')
			break;

		i++;
	}

	b[i] = 0;

	return i;
}

char* getHome (void){

	/*int p[2], ws;*/
	char* home = malloc (sizeof (char) * MAX_BUF);
/*
	createPipe (p);

	if (fork() == 0){
		close (p[0]);
		dup2 (p[1], 1);
		close (p[1]);

		execlp ("echo", "echo", "$HOME", NULL);
		_exit (-1);
	}
	else {
		close (p[1]);
		wait (&ws);

		readln (p[0], home, MAX_BUF);
		printf ("%s\n", home);
		close (p[0]);
	}*/

	return strcpy (home, getenv ("HOME"));
}

void createPipe (int p[]){
	
	if (pipe(p) == -1){
		perror ("pipe");
		_exit (-1);
	}
}

void writeln (int fd, char const *line, int size){

	if (write (fd, line, size) == -1){
		perror ("write");
		_exit (-1);
	}
}
