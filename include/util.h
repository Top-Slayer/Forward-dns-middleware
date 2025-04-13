#include "type.h"
#include <errno.h>

#ifndef UTIL_H
#define UTIL_H

#define LOG_ERR(msg) fprintf(stderr, "-> Error: [ %s:%d ]\n > custerr: %s\n > stderr: %s\n\n", __FILE__, __LINE__, msg, strerror(errno))

char *serve_js(char *content, char *type);
char *forward_url(char *url);
char *read_file(char *path, char *arg);
void getdns(FwDns **fwdns, int *size);
char getch();

#endif