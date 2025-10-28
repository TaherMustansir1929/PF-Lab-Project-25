#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H

#include "vector.h"
#define HEALTH_CHECK_URL "/api/health"

typedef struct UserSessions {
  char *user_id;
  Vector session_ids; // Vector of char* (session IDs)
} user_sessions_t;

typedef struct HealthCheckResponse {
  char *status;
  int active_user_count;
  int quiz_session_count;
  Vector quiz_session_ids; // Vector of user_sessions_t
  char *timestamp;
} health_check_response_t;

void health_check(void);

#endif // !HEALTH_CHECK_H
