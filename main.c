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

char *cached_js = NULL;

int make_socket_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main()
{
    getdns(&fwdns_obj, &size);
    cached_js = read_file("public/conn-dom.js", "rb");

    printf("Forwarding URL:\n");
    for (int i = 0; i < size; i++)
    {
        printf("http://localhost:%d%s -> %s\n", PORT, fwdns_obj[i].route, fwdns_obj[i].domain);
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        LOG_ERR("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        LOG_ERR("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0)
    {
        LOG_ERR("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    make_socket_non_blocking(server_fd);
    printf("Server is listening on port %d\n", PORT);

    fd_set master_set, read_fds;
    int fd_max = server_fd;
    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);

    while (1) {
        read_fds = master_set;
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        for (int fd = 0; fd <= fd_max; fd++) {
            if (!FD_ISSET(fd, &read_fds)) continue;

            if (fd == server_fd) {
                struct sockaddr_in client_addr;
                socklen_t addrlen = sizeof(client_addr);
                int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
                if (new_fd != -1) {
                    make_socket_non_blocking(new_fd);
                    FD_SET(new_fd, &master_set);
                    if (new_fd > fd_max) fd_max = new_fd;
                }
            } else {
                char buffer[BUFFER_SIZE] = {0};
                ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    close(fd);
                    FD_CLR(fd, &master_set);
                    continue;
                }

                printf("Received request:\n%s\n", buffer);

                int served = 0;

                HTTP_GET(buffer, "/conn-dom.js", {
                    if (cached_js) {
                        char *res = serve_js(cached_js, "application/javascript");
                        if (res) {
                            send(fd, res, strlen(res), 0);
                            free(res);
                            served = 1;
                        }
                    }
                });

                for (int i = 0; i < size && !served; i++) {
                    char temp[256];
                    snprintf(temp, sizeof(temp), "GET %s", fwdns_obj[i].route);
                    if (strstr(buffer, temp)) {
                        char *res = forward_url(fwdns_obj[i].domain);
                        if (res) {
                            send(fd, res, strlen(res), 0);
                            free(res);
                        }
                        break;
                    }
                }

                close(fd);
                FD_CLR(fd, &master_set);
            }
        }
    }

    free(fwdns_obj);
    free(cached_js);
    close(server_fd);
    return 0;
}
