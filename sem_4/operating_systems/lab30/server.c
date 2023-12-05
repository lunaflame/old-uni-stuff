#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "shared.h"

void strToUpper(char* str) {
	size_t cur = 0;

	while (str[cur] != 0) {
		str[cur] = toupper((int)str[cur]);
		cur++;
	}
}

void tryClose(int fd) {
	if (close(fd) == -1) {
		perror("close");
	}
}

int main(int argc, char *argv[]) {
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

	if (remove(SOCKET_PATH) == -1 && errno != ENOENT) {
		printf("[X] failed to remove %s", SOCKET_PATH);
		perror("");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

	if (bind(sfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("[X] bind");
		tryClose(sfd);
		return -1;
	}

	// mark socket as passive to receive connections (clients)
	if (listen(sfd, 1) == -1) {
		perror("[X] listen");
		tryClose(sfd);
		return -1;
	}

	ssize_t numRead;
	char buf[BUFSIZE + 1];
	buf[BUFSIZE + 1] = 0;

	printf("[ Waiting for a connection... ]\n");

	int cfd = accept(sfd, NULL, NULL);
	printf("[ Accepted socket: %d ]\n", cfd);


	while ((numRead = read(cfd, buf, BUFSIZE)) > 0) {
		// printf("read: %s\n", buf);
		buf[numRead] = 0;
		strToUpper(buf);
		printf("%s", buf);
	}

	if (numRead == -1) {
		perror("[X] read");
		tryClose(cfd);
		tryClose(sfd);
		return -1;
	}

	printf("[ Connection closed. ]\n");

	tryClose(cfd);
	tryClose(sfd);

}