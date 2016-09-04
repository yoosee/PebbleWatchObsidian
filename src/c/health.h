
#pragma once

extern void update_health(char *buffer);
extern void health_handler(HealthEventType event, void *context);
#define HEALTH_FMT_STR "%d"
#define MAX_HEALTH_STR "123456"   /* update to match HEALTH_FMT_STR */
