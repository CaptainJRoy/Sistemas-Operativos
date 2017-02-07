#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "sobu.h"

#define MAX_BUF 1024

char* getPWD (void){

	int p[2], ws, n;
	char* path = malloc (sizeof (char) * MAX_BUF);

	createPipe (p);

	if (fork() == 0){
		close (p[0]);
		dup2 (p[1], 1);
		close (p[1]);

		execlp ("pwd", "pwd", NULL);
		_exit (-1);
	}
	else {
		close (p[1]);
		wait (&ws);

		n = readln (p[0], path, MAX_BUF);
		close (p[0]);

		if (n > 0){
			path[n] = '/';
			path[n+1] = 0;
		}
		else return NULL;
	}
	return path;
}

void nada (){}

void naoexiste (){
	printf ("nao existe\n");
}

void handler_restore (){
	printf ("recuperado\n");
}

void handler_delete (){
	printf ("apagado\n");
}

int main(int argc, char const *argv[]){

	int i, ws, fifo, p[2];
	pid_t pid;
	char *path, *aux, *header;
	char file[MAX_BUF];

	path = getHome ();
	path = strcat (path, "/.Backup/fifo");

	fifo = open (path, O_WRONLY);

	free (path);

	path = getPWD ();
	header = malloc (sizeof (char) * MAX_BUF);

	pid = getpid();
	snprintf (header, MAX_BUF, "%d", pid);
	header = strcat (header, " ");

	if (argc >= 2){
		header = strcat (header, argv[1]);
		header = strcat (header, " ");
	}
	else {
		return 0;
	}

	aux = malloc (sizeof (char) * MAX_BUF);

	if (strcmp (argv[1], "backup") == 0){

		signal (SIGCONT, nada);

		for (i = 2; i < argc; i++){

			createPipe (p);

			if (fork() == 0){
				close (p[0]);
				close (fifo);
				dup2 (p[1], 1);
				close (p[1]);

				execlp ("find", "find", argv[i], "-type", "f", NULL);
				_exit (-1);
			}
			else {
				close (p[1]);
				wait (&ws);

				while (readln (p[0], file, MAX_BUF)){
					/* Substitui '\n' por NULL */
					/*file[n-1] = 0;*/

					aux = strcpy (aux, header);

					if (file[0] == '/'){
						aux = strcat (aux, file);
					}
					else {
						aux = strcat (aux, path);
						aux = strcat (aux, file);
					}

					writeln (fifo, aux, strlen (aux) + 1);

					/*Receber sinal*/
					pause();
					printf ("%s: copiado\n", file);
				}
			}
		}
	}
	else if (strcmp (argv[1], "restore") == 0){
		
		signal (SIGCONT, handler_restore);
		signal (SIGUSR1, naoexiste);

		for (i = 2; i < argc; i++){

			aux = strcpy (aux, header);

			if (argv[i][0] == '/'){
				aux = strcat (aux, argv[i]);
			}
			else {
				aux = strcat (aux, path);
				aux = strcat (aux, argv[i]);
			}

			writeln (fifo, aux, strlen (aux) + 1);

			/*Receber sinal*/
			printf ("%s: ", argv[i]);
			pause();
		}
	}
	else if (strcmp (argv[1], "delete") == 0){

		signal (SIGCONT, handler_delete);
		signal (SIGUSR1, naoexiste);

		for (i = 2; i < argc; i++){

			aux = strcpy (aux, header);
			aux = strcat (aux, argv[i]);

			writeln (fifo, aux, strlen (aux) + 1);
			
			printf ("%s: ", argv[i]);
			pause ();
		}
	}

	close (fifo);
	free (path);
	free (aux);

	return 0;
}
