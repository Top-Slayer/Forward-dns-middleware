#include "type.h"

#ifndef UTIL_H
#define UTIL_H

#define LOG_ERR(msg) fprintf(stderr, "-> Error: [ %s:%d ]\n > custerr: %s\n > stderr: %s\n\n", __FILE__, __LINE__, msg, strerror(errno))

char *custom_res(char *content, char *type);
char *read_file(char *path, char *arg);
void getdns(FwDns **fwdns);

#endif