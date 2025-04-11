#include "res.h"
#include <stdio.h>

char* custom_res(char *content, char *type) {
    size_t body_len = strlen(content);
    char *body = (char *)malloc(body_len + 1);
    if (!body) {
        perror("malloc failed");
        return NULL;
    }

    strcpy(body, content);

    char *type_content = (type == NULL) ? "text/plain" : type;
    const char *header_template = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Length: %zu\r\n"
        "Content-Disposition: inline; filename=\"public/output.js\"\r\n"
        "Connection: close\r\n"
        "\r\n";

    size_t header_len = strlen(header_template) + strlen(type_content) + 20;
    size_t response_len = header_len + body_len + 1;

    char *response = (char *)malloc(response_len);
    if (!response) {
        perror("malloc failed");
        free(body);
        return NULL;
    }

    sprintf(response, header_template, type_content, body_len);
    strcat(response, body);

    free(body);

    return response;
}