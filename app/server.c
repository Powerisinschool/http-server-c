#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUF_SIZE 500

// static const char *HTTP_STATUS = "HTTP/1.1 200 OK";
// static const char *HEADERS = "";
static const char *HTTP_200_RESPONSE = "HTTP/1.1 200 OK\r\n\r\n";
static const char *HTTP_404_RESPONSE = "HTTP/1.1 404 Not Found\r\n\r\n";

// char *respond(int status) {
// 	return "HTTP/1.1 " + "status" + " OK\r\n\r\n";
// }

int main() {
	// char *HTTP_RESPONSE;
	// sprintf(HTTP_RESPONSE, "%s\r\n%s\r\n", HTTP_STATUS, HEADERS);
	// Disable output buffering
	setbuf(stdout, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	//
	int server_fd, client_fd, client_addr_len;
	struct sockaddr_in client_addr;
	char buf[BUF_SIZE];
	char *path = malloc(sizeof(char) * 100);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting REUSE_PORT
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");

	client_addr_len = sizeof(client_addr);
	
	client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if (client_fd == -1) {
		printf("Failed to connect to the active client: %s...\n", strerror(errno));
		return 1;
	}
	printf("Client connected\n");

	/* Handle receiving data */
	if (recv(client_fd, buf, BUF_SIZE, 0) == -1) {
		printf("Receiving failed: %s \n", strerror(errno));
	}

	// printf("\n=====\n%s\n=====\n", buf);

	strtok(buf, " ");
	path = strtok(NULL, " ");
	// printf("Path: %s\n", path);

	if (strcmp(path, "/") == 0) {
		// printf("200 OK\n");
		write(client_fd, (const char *)HTTP_200_RESPONSE, strlen(HTTP_200_RESPONSE));
	} else {
		write(client_fd, (const char *)HTTP_404_RESPONSE, strlen(HTTP_404_RESPONSE));
	}
	
	
	close(server_fd);

	return 0;
}
