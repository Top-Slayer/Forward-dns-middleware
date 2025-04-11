#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "util.h"
#include "type.h"

char *read_file(char *path, char *arg)
{
    FILE *fp = fopen(path, arg);
    if (fp == NULL)
    {
        perror("File not found");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char *data = (char *)malloc(len + 1);
    if (data == NULL)
    {
        perror("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    fread(data, 1, len, fp);
    data[len] = '\0';

    fclose(fp);

    return data;
}

void getdns(FwDns **fwdns)
{
    char *data = read_file("misc/dnsmap.json", "r");

    cJSON *root = cJSON_Parse(data);
    if (!root)
    {
        perror("JSON parse error\n");
        free(data);
        return;
    }

    *fwdns = (FwDns *)malloc(sizeof(FwDns) * cJSON_GetArraySize(root));

    cJSON *current_element = NULL;
    int index = 0;
    cJSON_ArrayForEach(current_element, root)
    {
        (*fwdns)[index].route = strdup(current_element->string);
        (*fwdns)[index].domain = strdup(current_element->valuestring);
        index++;
    }

    cJSON_Delete(root);
    free(data);
}