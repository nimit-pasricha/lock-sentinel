#ifndef CONFIG_H
#define CONFIG_H

typedef enum
{
    RETURN,  // Don't lock. Return EDEADLK
    FREEZE,  // Default. Report the deadlock, and let it happen
    WAIT_DIE // If deadlock about to occur, put called thread to sleep until safe
} sentinel_policy_t;

typedef struct
{
    sentinel_policy_t policy;
    char graph_file_path[256];
} sentinel_config_t;

extern sentinel_config_t global_config;

// load settings from sentinel.ini
void load_config();

#endif
