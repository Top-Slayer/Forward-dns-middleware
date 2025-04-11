#ifndef UTIL_H
#define UTIL_H

#include "type.h"

char *custom_res(char *content, char *type);
char *read_file(char *path, char *arg);
void getdns(FwDns **fwdns);

#endif