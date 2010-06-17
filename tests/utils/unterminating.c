#include <unistd.h>
#include <signal.h>
#include <stdio.h>

void handler(int signum) {

}

int main(int argc, char* argv[]) {
	signal(SIGTERM, handler);

	if (argc == 3) {
		fprintf(stdout, "%s", argv[1]);
		fprintf(stderr, "%s", argv[2]);
		fflush(NULL);
		while (1) sleep(1);
	} else {
		return 1;
	}

	return 0;
}
