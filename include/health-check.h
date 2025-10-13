#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H

#include "vector.h"
#define HEALTH_CHECK_URL "/api/health"

typedef struct HealthCheckResponse {
  char *status;
  int active_sessions_count;
  Vector active_session_ids;
  char *timestamp;
} health_check_response_t;

void health_check(void);

#endif // !HEALTH_CHECK_H
