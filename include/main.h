#ifndef MAIN_H
#define MAIN_H

#include <cjson/cJSON.h>
#include <curl/curl.h>

#define HOST "host.docker.internal"
#define BASE_URL "https://pf-lab-project-25-python-api.vercel.app"

typedef struct State {
  char *session_id;
  char *course;
  char *topic;
} state_t;

#endif
