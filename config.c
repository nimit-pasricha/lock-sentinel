#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

sentinel_config_t global_config = {.policy = FREEZE};

void load_config() {
    FILE *fp = fopen("sentinel.ini", "r");
    if (!fp) {
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Remove leading whitespace
        char *ptr = line;
        while (*ptr && isspace((unsigned char)*ptr)) {
            ptr++;
        };

        // Skip Empty lines or Comments
        if (*ptr == '\0' || *ptr == ';') {
            continue;
        }
        line[strcspn(line, "\r\n")] = 0;
        if (strncmp(ptr, "policy=", 7) == 0) {
            char *value = ptr + 7; // skip "policy="
            if (strcmp(value, "return") == 0) {
                global_config.policy = RETURN;
            } else if (strcmp(value, "freeze") == 0) {
                global_config.policy = FREEZE;
            } else if (strcmp(value, "stall") == 0) {
                global_config.policy = WAIT_DIE;
            }
        }
    }
    fclose(fp);
}
