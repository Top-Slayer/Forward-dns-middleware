#ifndef HANDLE_H
#define HANDLE_H

#define HTTP_GET(buf_req, endpoint, block)   \
    do                                       \
    {                                        \
        if (strstr(buffer, "GET " endpoint)) \
        {                                    \
            block                            \
        }                                    \
    } while (0)

#define LOOP_HTTP_GET(buf_req, obj, endpoint)                    \
    for (int i = 0; i < size; i++)                               \
    {                                                            \
        {                                                        \
            char *response = forward_url(obj[i].domain);         \
            if (response != NULL)                                \
            {                                                    \
                send(new_socket, response, strlen(response), 0); \
                free(response);                                  \
            }                                                    \
        }                                                        \
    }

#endif