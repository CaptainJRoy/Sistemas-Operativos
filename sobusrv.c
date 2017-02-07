#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#include "sobu.h"

#define MAX_BUF 1024

char* getDigest (char* path){

	int p[2], ws;
	char result[MAX_BUF];
	char *token;

	createPipe (p);

	if (fork() == 0){
		close (p[0]);
		dup2 (p[1], 1);
		close (p[1]);

		execlp ("sha1sum", "sha1sum", path, NULL);
		_exit (-1);
	}
	else {
		close (p[1]);
		wait (&ws);

		readln (p[0], result, MAX_BUF);
		close (p[0]);
	}
	
	token = strtok (result, " ");

	return token;
}

char* separateLast (char* path){
	
	int i, j, n;
	char *file;

	file = malloc (sizeof (char) * MAX_BUF);

	n = strlen (path);

	for (i = n - 1; i >= 0 && path[i] != '/'; i--);

	for (j = 0, i++; i+j < n; j++){

		file[j] = path[i+j];
	}
	path[i] = 0;
	file[j] = 0;

	return file;
}

void backup (char* path){

	int ws, p[2], fdata;
	char *digest, *file, *data, *metadata;
	/*char nl = '\n';*/

	digest = malloc (sizeof (char) * MAX_BUF);
	digest = strcpy (digest, getDigest (path));

	data = malloc (sizeof (char) * MAX_BUF);
	data = strcpy (data, "data/");
	data = strcat (data, digest);

	fdata = open (data, O_WRONLY);
	if (fdata == -1){

		fdata = open (data, O_CREAT | O_WRONLY, 0666);

		createPipe (p);

		if (fork() == 0){
			close (p[0]);
			dup2 (p[1], 1);
			close (p[1]);

			execlp ("cat", "cat", path, NULL);

			_exit (-1);
		}
		else {
			close (p[1]);
		}

		if (fork() == 0){
			dup2 (p[0], 0);
			close (p[0]);
			dup2 (fdata, 1);
			close (fdata);

			execlp ("gzip", "gzip", "-f", NULL);

			_exit (-1);
		}
		else {
			close (p[0]);
			close (fdata);
			wait (&ws);
			wait (&ws);
		}
	}
	/* Escrever no metadata */
	file = separateLast (path);

	metadata = malloc (sizeof (char) * MAX_BUF);
	metadata = strcpy (metadata, "metadata/");
	/*dest = strcat (dest, path);
	
	if (fork() == 0){

		execlp ("mkdir", "mkdir", "-p", dest, NULL);

		_exit (-1);
	}
	else {

		wait (&ws);
	}
	*/
	metadata = strcat (metadata, file);
	free (file);
	data = strcpy (data, "../data/");
	data = strcat (data, digest);

	if (fork() == 0){

		execlp ("ln", "ln", "-sf", data, metadata, NULL);
		_exit (-1);
	}
	else {
		wait (&ws);
	}
	/*	
	fmetadata = open (dest, O_CREAT | O_TRUNC | O_WRONLY, 0666);

	writeln (fmetadata, digest, strlen (digest));*/
/*
	if (write (fmetadata, digest, strlen(digest)) == -1)
		|| write (fmetadata, &nl, 1) == -1
		|| write (fmetadata, path, strlen (path)) == -1)
	{

		perror ("write");
		free (digest);
		free (dest);
		close (fmetadata);
		_exit (-1);
	}
*/	
	free (digest);
	free (data);
	free (metadata);
	/*close (fmetadata);*/
}

int restore (char* path){

	int ws, fmetadata, fdest;
	char *file, *metadata;

	file = separateLast (path);

	metadata = malloc (sizeof (char) * MAX_BUF);
	metadata = strcpy (metadata, "metadata/");
	metadata = strcat (metadata, file);

	fmetadata = open (metadata, O_RDONLY);
	if (fmetadata == -1)
		return -1;

/*
	n = readln (fmetadata, digest, MAX_BUF);
	digest [n] = 0;

	close (fmetadata);

	dest = strcpy (dest, "data/");
	dest = strcat (dest, digest);

	fdata = open (dest, O_RDONLY);*/

	path = strcat (path, file);

	fdest = open (path, O_CREAT | O_TRUNC | O_WRONLY, 0666);

	if (fork() == 0){

		dup2 (fmetadata, 0);
		close (fmetadata);
		dup2 (fdest, 1);
		close (fdest);

		execlp ("gzip", "gzip", "-fd", NULL);
		_exit (-1);
	}
	else {
		wait (&ws);
	}
	
	/*Enviar sinal*/

	close (fmetadata);
	close (fdest);
	return 0;
}

int delete (char* arg){

	int ws, fd;
	char *path = malloc (sizeof (char) * MAX_BUF);

	path = strcpy (path, "metadata/");
	path = strcat (path, arg);

	fd = open (path, O_RDONLY);
	if (fd == -1)
		return -1;

	close (fd);

	if (fork() == 0){

		execlp ("rm", "rm", path, NULL);

		_exit (-1);
	}
	else{
		wait (&ws);
	}

	free (path);
	return 0;
}

void changeDirectory (void){

	char *path;

	path = getHome ();
	path = strcat (path, "/.Backup");

	if (chdir (path) == -1){
		perror ("chdir");
		_exit (-1);
	}

	free (path);
}

int main(){

	int fifo;
	pid_t pid;
	char arg[MAX_BUF];
	char *token;

	if (fork() == 0){

		changeDirectory ();
/*
		if (fork() == 0){
			execlp ("rm", "rm", "-f", "fifo", NULL);
			_exit (-1);
		}
		else {
			wait (&ws);
		}
*/
		mkfifo ("fifo", 0666);

		while (1){
	
			fifo = open ("fifo", O_RDONLY);

			if (readln (fifo, arg, MAX_BUF) > 0){

				token = strtok (arg, " ");
				pid = atoi (token);

				token = strtok (NULL, " ");

				if (strcmp (token, "backup") == 0){
					token = strtok (NULL, "");
					backup (token);
					kill (pid, SIGCONT);
				}
				else if (strcmp (token, "restore") == 0){
					token = strtok (NULL, "");
					if (restore (token) == -1){
						kill (pid, SIGUSR1);
					}
					else
						kill (pid, SIGCONT);
				}
				else if (strcmp (token, "delete") == 0){
					token = strtok (NULL, "");
					if (delete (token) == -1){
						kill (pid, SIGUSR1);
					}
					else
						kill (pid, SIGCONT);
				}
			}
			close (fifo);
		}
		_exit(0);
	}

	return 0;
}