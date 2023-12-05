#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "shared.h"

int main(int argc, char** argv) {
	struct sockaddr_un addr;
	
	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sfd == -1) {
		perror("[X] socket");
		return -1;
	}

	// apparently this is a thing
	if (strlen(SOCKET_PATH) > sizeof(addr.sun_path) - 1) {
		printf("[X] Server socket path too long: %s (%ld, max: %ld)",
			SOCKET_PATH, strlen(SOCKET_PATH), sizeof(addr.sun_path) - 1);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

	if (connect(sfd, (struct sockaddr*) &addr,
							sizeof(struct sockaddr_un)) == -1) {
		perror("[X] connect");
		return -1;
	}


	printf("[ Connected. ]\n");

	// writing to a dead socket will give you a SIGPIPE which kills the program
	// which is ebin (NOT)

	signal(SIGPIPE, SIG_IGN);

	ssize_t numRead;
	char buf[BUFSIZE];

	while ((numRead = read(STDIN_FILENO, buf, BUFSIZE)) > 0) {
		ssize_t wroteAmt = write(sfd, buf, numRead);

		if (wroteAmt == -1) {
			perror("[X] write");
			printf("[ Disconnected. ]\n");
			return -1;
		} else if (wroteAmt != numRead) {
			printf("[!] partial/failed write (wrote %ld, %ld expected)\n", wroteAmt, numRead);
		}
	}

	if (numRead == -1) {
		perror("[X] read");
		return -1;
	}
}