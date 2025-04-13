#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <dirent.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include "util.h"
#include "type.h"

char *read_file(char *path, char *arg)
{
    FILE *fp = fopen(path, arg);
    if (fp == NULL)
    {
        LOG_ERR("File not found");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char *data = (char *)malloc(len + 1);
    if (data == NULL)
    {
        LOG_ERR("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    fread(data, 1, len, fp);
    data[len] = '\0';

    fclose(fp);

    return data;
}

void getdns(FwDns **fwdns, int *size)
{
    DIR *dir = opendir("misc");
    if (dir)
    {
        closedir(dir);
    }
    else
    {
        if (mkdir("misc", 0777) == 0)
        {
            printf("Directory created successfully.\n");
        }
        else
        {
            LOG_ERR("Failed to create directory");
            return;
        }
    }

    char *data = read_file("misc/dnsmap.json", "r");

    cJSON *root = cJSON_Parse(data);
    if (!root)
    {
        LOG_ERR("JSON parse error");
        free(data);
        exit(1);
    }

    *size = cJSON_GetArraySize(root);
    *fwdns = (FwDns *)malloc(sizeof(FwDns) * (*size));

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

char getch()
{
    struct termios oldt, newt;
    char c;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    read(STDIN_FILENO, &c, 1);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
}