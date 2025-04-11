#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include "res.h"

#define PORT 8080
#define BUFFER_SIZE 2048

char* serve_code() {
	FILE *fptr = fopen("public/output.js", "rb");

	if (fptr == NULL) {
		perror("File not found");
 		return NULL;
	}

	fseek(fptr, 0, SEEK_END);
    long filesize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

	char *buf = (char*)malloc(filesize + 1); 
    if (buf == NULL) {
		perror("Memory allocation failed");
        fclose(fptr);
        return NULL;
    }

    fread(buf, 1, filesize, fptr);
	buf[filesize] = '\0';

    fclose(fptr);

    return buf;
}

void send_in_chunks(int socket, const char *response, size_t len) {
    size_t total_sent = 0;
    while (total_sent < len) {
        ssize_t sent = send(socket, response + total_sent, len - total_sent, 0);
        if (sent < 0) {
            perror("send failed");
            break;
        }
        total_sent += sent;
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Binding the socket to the port 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listening for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        // Accept connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Read the request
        ssize_t bytes_read = read(new_socket, buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            perror("read failed");
            close(new_socket);
            continue;
        }

        printf("Received request:\n%s\n", buffer);

        // Check if the request is for the /output.js file
        if (strstr(buffer, "GET /output.js") != NULL) {
            // Serve the file
            char *content = serve_code();
            if (content != NULL) {
                char *response = custom_res(content, "application/javascript");
                if (response != NULL) {
                    send_in_chunks(new_socket, response, strlen(response));
                    printf("Response sent for /output.js\n");
                    free(response);
                }
                free(content);
            }
        }

        close(new_socket);
    }

    return 0;
}
