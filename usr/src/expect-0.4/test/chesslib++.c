/* testlib.c for c++ - test expectlib */

#include <stdio.h>
#include "expect.h"

extern "C" {
	extern exit(...);
	extern int write(...);
	extern int strlen(...);
}

timedout()
{
	fprintf(stderr,"timed out\n");
	exit(-1);
}

char move[100];

read_first_move(int fd)
{
	if (EXP_TIMEOUT == exp_expectl(fd,"first\r\n1.*\r\n",0,(char *)0)) {
		timedout();
	}
	sscanf(exp_match,"%*s 1. %s",move);
}

/* moves and counter-moves are printed out in different formats, sigh... */

read_counter_move(int fd)
{
	switch (exp_expectl(fd,"*...*\r\n",0,(char *)0)) {
	case EXP_TIMEOUT: timedout();
	case EXP_EOF: exit(-1);
	}

	sscanf(exp_match,"%*s %*s %*s %*s ... %s",move);
}

read_move(int fd)
{
	switch (exp_expectl(fd,"*...*\r\n*.*\r\n",0,(char *)0)) {
	case EXP_TIMEOUT: timedout();
	case EXP_EOF: exit(-1);
	}

	sscanf(exp_match,"%*s %*s ... %*s %*s %s",move);
}

send_move(int fd)
{
	write(fd,move,strlen(move));
}

main(){
	int fd1, fd2;

	exp_loguser = 1;
	exp_timeout = 3600;

	fd1 = exp_spawnl("chess","chess",(char *)0);

	if (-1 == exp_expectl(fd1,"Chess\r\n",0,(char *)0)) exit;

	if (-1 == write(fd1,"first\r",6)) exit;

	read_first_move(fd1);

	fd2 = exp_spawnl("chess",0);

	if (-1 == exp_expectl(fd2,"Chess\r\n",0,(char *)0)) exit;

	for (;;) {
		send_move(fd2);
		read_counter_move(fd2);

		send_move(fd1);
		read_move(fd1);
	}
}
