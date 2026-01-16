#include "config.h"
#include <stdio.h>
#include <string.h>

sentinel_config_t global_config = {.policy = FREEZE};

void load_config() {
    FILE *fp = fopen("sentinel.conf", "r");
    if (!fp) {
        fprintf(stderr, "[WARNING] load_config: no config file found. Using default: freeze");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strncmp(line, "policy=", 7) == 0) {
            char *value = line + 7; // skip "policy="
            if (strcmp(value, "return") == 0) {
                global_config.policy = RETURN;
            } else if (strcmp(value, "freeze") == 0) {
                global_config.policy = FREEZE;
            } else if (strcmp(value, "stall") == 0) {
                global_config.policy = STALL;
            } else {
                fprintf(stderr, "[WARNING] Unknown policy '%s'. Using default: freeze.\n", value);
            }
        }
    }
    fclose(fp);
}
