#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

sentinel_config_t global_config = {.policy = FREEZE, .graph_file_path = ""};

void load_config()
{
    FILE *fp = fopen("sentinel.ini", "r");
    if (!fp)
    {
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        // Remove leading whitespace
        char *ptr = line;
        while (*ptr && isspace((unsigned char)*ptr))
        {
            ptr++;
        };

        // Skip Empty lines or Comments
        if (*ptr == '\0' || *ptr == ';')
        {
            continue;
        }
        line[strcspn(line, "\r\n")] = 0;
        if (strncmp(ptr, "policy=", 7) == 0)
        {
            char *value = ptr + 7; // skip "policy="
            if (strcmp(value, "return") == 0)
            {
                global_config.policy = RETURN;
            }
            else if (strcmp(value, "freeze") == 0)
            {
                global_config.policy = FREEZE;
            }
            else if (strcmp(value, "wait_die") == 0)
            {
                global_config.policy = WAIT_DIE;
            }
        }
        else if (strncmp(ptr, "graph_file_path=", 16) == 0)
        {
            char *val = ptr + 16;
            strncpy(global_config.graph_file_path, val, 255);
            global_config.graph_file_path[255] = '\0'; // Safety null
        }
    }
    fclose(fp);
}
