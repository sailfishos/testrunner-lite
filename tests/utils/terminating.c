#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
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
