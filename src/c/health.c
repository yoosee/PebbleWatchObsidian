#include <pebble.h>
#include "health.h"

void update_health(char *buffer) {
    //static char buffer[] = MAX_HEALTH_STR;
    time_t start = time_start_of_today();
    time_t end = time(NULL);  /* Now */

    /* Check data is available */
    HealthServiceAccessibilityMask result = health_service_metric_accessible(HealthMetricStepCount, start, end);
    if(result & HealthServiceAccessibilityMaskAvailable) {
        HealthValue steps = health_service_sum(HealthMetricStepCount, start, end);

        APP_LOG(APP_LOG_LEVEL_INFO, "Steps today: %d", (int)steps);
        snprintf(buffer, sizeof(MAX_HEALTH_STR), HEALTH_FMT_STR, (int) steps);
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "No data available");
        strcpy(buffer, "");
    }
    // text_layer_set_text(health_tlayer, buffer);
}

void health_handler(HealthEventType event, void *context) {
    // Which type of event occured?
    switch(event)
    {
        case HealthEventSignificantUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventSignificantUpdate event");
            //break;
        case HealthEventMovementUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventMovementUpdate event");
//            update_health();
            break;
        case HealthEventSleepUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventSleepUpdate event");
            break;        
      case HealthEventMetricAlert:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventMetricAlert event");
            break;  
      case HealthEventHeartRateUpdate:
            APP_LOG(APP_LOG_LEVEL_INFO, "New HealthService HealthEventHeartRateUpdate event");
            break;
    }
}

void cleanup_health() {
    health_service_events_unsubscribe();
}
