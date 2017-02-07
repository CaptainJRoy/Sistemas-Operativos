CFLAGS=-Wall -Wextra -pedantic -O2 -g

sobu:
	$(CC) $(CFLAGS) -o sobusrv sobusrv.c sobu.c
	$(CC) $(CFLAGS) -o sobucli sobucli.c sobu.c
	./instalation.sh

clean: 
	rm sobusrv sobucli
