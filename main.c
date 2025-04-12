#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "util.h"
#include "type.h"
#include "handle.h"

#define PORT 8080
#define BUFFER_SIZE 2048

FwDns *fwdns_obj = NULL;
int size = 0;

int main()
{
    getdns(&fwdns_obj, &size);

    printf("Forwarding URL:\n");
    for (int i = 0; i < size; i++)
    {
        printf("http://localhost:%d%s -> %s\n", PORT, fwdns_obj[i].route, fwdns_obj[i].domain);
    }
    

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        LOG_ERR("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        LOG_ERR("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0)
    {
        LOG_ERR("listen failed");
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
            LOG_ERR("Accept failed");
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_read = read(new_socket, buffer, BUFFER_SIZE);
        if (bytes_read < 0)
        {
            LOG_ERR("read failed");
            close(new_socket);
            continue;
        }

        printf("Received request:\n%s\n", buffer);

        // send connect into middleware js file
        HTTP_GET(buffer, "/conn-dom.js", {
            char *content = read_file("public/conn-dom.js", "rb");
            if (content != NULL)
            {
                char *response = serve_js(content, "application/javascript");
                if (response != NULL)
                {
                    send(new_socket, response, strlen(response), 0);
                    free(response);
                }
                free(content);
            }
        });

        // loop forwarding url
        for (int i = 0; i < size; i++)
        {
            char temp[256];
            snprintf(temp, sizeof(temp), "GET %s", fwdns_obj[i].route);
            if (strstr(buffer, temp))
            {
                char *response = forward_url(fwdns_obj[i].domain);
                if (response != NULL)
                {
                    send(new_socket, response, strlen(response), 0);
                    free(response);
                }
            }
        }

        close(new_socket);
    }

    free(fwdns_obj);
    return 0;
}
