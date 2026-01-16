#ifndef CONFIG_H
#define CONFIG_H

typedef enum {
    RETURN, // Don't lock. Return EDEADLK
    FREEZE, // Report the deadlock, and let it happen
    STALL   // If deadlock about to occur, put called thread to sleep until safe
    // TODO: priority based reordering
} sentinel_policy_t;

typedef struct {
    sentinel_policy_t policy;
    // TODO: add log file paths
} sentinel_config_t;

extern sentinel_config_t global_config;

// load settings from sentinel.conf
void load_config();

#endif
