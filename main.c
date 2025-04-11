#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include "util.h"
#include "type.h"

#define PORT 8080
#define BUFFER_SIZE 2048

FwDns *fwdns_obj = NULL;

int main()
{
    getdns(&fwdns_obj);

    printf("%s\n", fwdns_obj[0].domain);

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_read = read(new_socket, buffer, BUFFER_SIZE);
        if (bytes_read < 0)
        {
            perror("read failed");
            close(new_socket);
            continue;
        }

        printf("Received request:\n%s\n", buffer);

        if (strstr(buffer, "GET /output.js") != NULL)
        {
            char *content = read_file("public/output.js", "rb");
            if (content != NULL)
            {
                char *response = custom_res(content, "application/javascript");
                if (response != NULL)
                {
                    send(new_socket, response, strlen(response), 0);
                    printf("Response sent for /output.js\n");
                    free(response);
                }
                free(content);
            }
        }

        close(new_socket);
    }

    free(fwdns_obj);
    return 0;
}
