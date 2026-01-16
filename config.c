#include "config.h"

#include <stdio.h>

sentinel_config_t global_config = {.policy = FREEZE};

void load_config() {
    FILE *fp = fopen("sentinel.conf", "r");
    if (!fp) {
        fprintf(stderr, "[WARNING] load_config: no config file found. Using default: FREEZE");
        return;
    }
}
