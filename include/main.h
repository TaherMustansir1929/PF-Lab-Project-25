#ifndef MAIN_H
#define MAIN_H

#include <curl/curl.h>
#include <cjson/cJSON.h>

#define HOST "host.docker.internal"
#define BASE_URL "http://localhost:8000"

typedef struct State {
  char* session_id;
  char* course;
  char* topic;
} state_t ;

#endif
