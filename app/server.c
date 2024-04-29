#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_SIZE 500

static const char *HTTP_200_RESPONSE = "HTTP/1.1 200 OK\r\n\r\n";
static const char *HTTP_404_RESPONSE = "HTTP/1.1 404 Not Found\r\n\r\n";

void *handle_connection(void *arg) {
	int client_fd = *((int *)arg);
    char buf[BUF_SIZE];
	char *buf2 = malloc(sizeof(char) * BUF_SIZE);
    char *path;
	char *response = malloc(sizeof(char)*500);
	char *userAgent = malloc(sizeof(char)*500);

	if (recv(client_fd, buf, BUF_SIZE, 0) == -1) {
		printf("Receiving failed: %s \n", strerror(errno));
		close(client_fd);
		return NULL;
	}

	strcpy(buf2, buf);
	
	for (char *header = strtok(buf2, "\r\n"); header != NULL; header = strtok(NULL, "\r\n")) {
		// printf("Header Value: %s\n\n", header);
		if (strncmp(header, "User-Agent", 10) == 0) {
			userAgent = header + 12;
			// printf("User Agent: %s\n", userAgent);
		}
	}

	if (strtok(buf, " ") == NULL) {
        printf("Invalid request\n");
        close(client_fd);
        return NULL;
    }
	path = strtok(NULL, " ");

	if (strncmp(path, "/echo/", 6) == 0) {
		const char *content = path + 6;
		sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s", strlen(content), content);
	} else if (strcmp(path, "/user-agent") == 0) {
		sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s", strlen(userAgent), userAgent);
	} else if (strcmp(path, "/") == 0) {
		response = (char *)HTTP_200_RESPONSE;
	} else {
		response = (char *)HTTP_404_RESPONSE;
	}
	write(client_fd, (const char *)response, strlen(response));

	close(client_fd);
	return NULL;
}

int main() {
	// char *HTTP_RESPONSE;
	// sprintf(HTTP_RESPONSE, "%s\r\n%s\r\n", HTTP_STATUS, HEADERS);
	// Disable output buffering
	setbuf(stdout, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	//
	int server_fd, client_fd;
	struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t tid;
	// char buf[BUF_SIZE];
	// char *buf2 = malloc(sizeof(char) * BUF_SIZE);
	// char *header = malloc(sizeof(char) * 100);
	// char *response = malloc(sizeof(char)*500);
	// char *userAgent = malloc(sizeof(char)*500);
	// char *path = malloc(sizeof(char) * 100);

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
	
	printf("Waiting for clients to connect...\n");

	while(1) {
		client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		if (client_fd == -1) {
			printf("Failed to connect to the active client: %s...\n", strerror(errno));
			continue;
		}
		if (pthread_create(&tid, NULL, handle_connection, &client_fd) != 0) {
            printf("Failed to create thread for handling connection\n");
            close(client_fd);
        }
		printf("Client connected\n");
	}

	/* Handle receiving data */
	// if (recv(client_fd, buf, BUF_SIZE, 0) == -1) {
	// 	printf("Receiving failed: %s \n", strerror(errno));
	// 	return 1;
	// }

	// printf("\n=====\n%s\n=====\n", buf);

	// strcpy(buf2, buf);
	
	// for (header = strtok(buf2, "\r\n"); header != NULL; header = strtok(NULL, "\r\n")) {
	// 	// printf("Header Value: %s\n\n", header);
	// 	if (strncmp(header, "User-Agent", 10) == 0) {
	// 		userAgent = header + 12;
	// 		// printf("User Agent: %s\n", userAgent);
	// 	}
	// }

	// strtok(buf, " ");
	// path = strtok(NULL, " ");
	// printf("Path: %s\n", path);

	// if (strncmp(path, "/echo/", 6) == 0) {
	// 	const char *content = path + 6;
	// 	// response = (char *)HTTP_200_RESPONSE;
	// 	sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s", strlen(content), content);
	// } else if (strcmp(path, "/user-agent") == 0) {
	// 	sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s", strlen(userAgent), userAgent);
	// } else if (strcmp(path, "/") == 0) {
	// 	// printf("200 OK\n");
	// 	response = (char *)HTTP_200_RESPONSE;
	// } else {
	// 	response = (char *)HTTP_404_RESPONSE;
	// }
	// write(client_fd, (const char *)response, strlen(response));
	
	
	close(server_fd);

	return 0;
}
