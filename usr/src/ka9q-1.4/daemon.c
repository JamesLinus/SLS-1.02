/*	Detach a daemon process from login session context */

#include <signal.h>
#include <stdio.h>

#if	(defined(SUNOS4) || defined(BSD))
#include <sys/param.h>
#define	_NFILE NOFILE
#endif
#define _NFILE 20

#ifdef	SYSV
/* extern void exit(); -- seems to be builtin to GCC */
#else
/* extern int exit(); */
#endif

void
daemon()
{
	int fd;
	FILE *fp;			/* pointer to current descriptor */	

	/* if started by init there's no need to detach */

	if (getppid() == 1)
		goto out;

	/* ensure process is not a process group leader */

	if (fork() != 0)
		exit(0);	/* parent */

	/* child */

	(void)setpgrp();		/* lose ctrl term, chg proc grp */

	(void)signal(SIGHUP, SIG_IGN);	/* immune from pgrp death */

	if (fork() != 0)	/* become non pgrp leader */
		exit(0);	/* first child */

	/* close all file descriptors */

out:

	_cleanup();

	for (fd = 0; fd < _NFILE; fd++) {
		(void)close(fd);
	}

/*	(void)chdir("/");		/* move off any mounted file system */

	(void)umask(0);

	return;
}
