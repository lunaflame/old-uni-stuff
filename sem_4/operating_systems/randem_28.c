#include <stdlib.h>
#include <time.h>
#include <libgen.h>
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

const int Amount = 100;

struct popen2 {
    pid_t child_pid;
    FILE *from_child, *to_child;
    int from_fd, to_fd;
};

int popen2(const char *cmdline, struct popen2 *childinfo) {
    pid_t p;
    int in[2], out[2];

    if (pipe(in)) return -1;
    if (pipe(out)) return -1;

    //printf("in[0] = %d, in[1] = %d\n", in[0], in[1]);
    //printf("out[0] = %d, out[1] = %d\n", out[0], out[1]);

    p = fork();
    if(p < 0) return p; /* Fork failed */
    if(p == 0) { /* child */
        dup2(in[0], 0);
        dup2(out[1], 1);
        close(in[1]);
        close(out[0]);
        
        execl("/bin/sh", "sh", "-c", cmdline, NULL);
        perror("execl"); exit(99);
    }
    childinfo->child_pid = p;
    childinfo->to_child = fdopen(in[1], "w");
    childinfo->from_child = fdopen(out[0], "r");
    childinfo->to_fd = in[1];
    childinfo->from_fd = out[0];

    close(in[1]);
    close(out[0]);
    return 0; 
}

int main() {
	srand(time(NULL)); // seed

	int random[Amount];

	for (int i = 0; i < Amount; i++) {
		random[i] = rand();
	}

	struct popen2 pipes;
	int pid = popen2("echo hi hello", &pipes);

	if (pid < 0) {
		printf("owned");
		return pid;
	}

	FILE* in = pipes.from_child;
	FILE* out = pipes.to_child;
	printf("sticking in\n");
	for (int i = 0; i < Amount; i++) {
		fprintf(in, "%d ", random[i]);
	}
	fprintf(in, "\n");
	fclose(in);
	close(pipes.to_fd);
	wait(NULL); close(pipes.from_fd);
	printf("reading out\n");

	char buf[1024];
    int nbytes;
    while ((nbytes = read(pipes.from_fd, buf, sizeof(buf))) > 0)
        printf("xd %.*s", nbytes, buf);

    printf("%d\n", nbytes);
	/*int temp = 0;
	for (int i = 0; i < Amount; i++) {
		fscanf(in, "%d", &temp);
		fprintf(out, "%d", temp);
	}*/
	// fscanf("%s");
}