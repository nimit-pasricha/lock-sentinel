#ifndef CONFIG_H
#define CONFIG_H

typedef enum {
    RETURN,    // Don't lock. Return EDEADLK
    FREEZE,    // Report the deadlock, and let it happen
    STALL      // If deadlock about to occur, put called thread to sleep until safe
    // TODO: priority
} sentinel_policy_t;

#endif
