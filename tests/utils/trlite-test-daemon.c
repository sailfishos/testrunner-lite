#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main() {
	int errnum;

	/* Daemonize */
	if (daemon(0,0) < 0) {
		errnum = errno;
		fprintf(stderr, "Daemonizing failed: %s\n", strerror(errnum));
		return -1;
	}

	while (1) {
		sleep(1);
	}

	return 0;
}
