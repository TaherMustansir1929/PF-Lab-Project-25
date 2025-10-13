#include "health-check.h"
#include "main.h"
#include "requests.h"
#include "vector.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

health_check_response_t parse_response(const char *json_str) {
  health_check_response_t result = {0};
  vector_init(&result.active_session_ids, sizeof(char *));
  cJSON *root = cJSON_Parse(json_str);
  if (!root) {
    perror("Failed to parse quiz_start_response\n");
    return result;
  }

  cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
  cJSON *active_sessions_count =
      cJSON_GetObjectItemCaseSensitive(root, "active_sessions_count");
  cJSON *active_sessions_ids =
      cJSON_GetObjectItemCaseSensitive(root, "active_sessions_ids");
  cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(root, "timestamp");

  if (cJSON_IsString(status))
    result.status = strdup(status->valuestring);
  if (cJSON_IsNumber(active_sessions_count))
    result.active_sessions_count = active_sessions_count->valueint;
  if (cJSON_IsString(timestamp))
    result.timestamp = strdup(timestamp->valuestring);

  if (cJSON_IsArray(active_sessions_ids)) {
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, active_sessions_ids) {
      if (cJSON_IsString(item) && (item->valuestring != NULL)) {
        char *string_copy = strdup(item->valuestring);
        if (string_copy == NULL) {
          fprintf(stderr, "strdup failed: out of memory\n");
          cJSON_Delete(active_sessions_ids);
          vector_free(&result.active_session_ids);
          return result;
        }
        vector_push_back(&result.active_session_ids, &string_copy);
      }
    }
  }

  cJSON_Delete(root);
  return result;
}

void health_check(void) {
  char url[256];
  snprintf(url, sizeof(url), "%s%s", BASE_URL, HEALTH_CHECK_URL);
  memory_t chunk = get_request(url);

  if (chunk.err != NULL) {
    fprintf(stderr, "Failed to recieve response: %s\n", chunk.err);
  } else {
    // DEBUG PRINT
    printf("\n\nRAW CHUNK RESPONSE: %s\n\n", chunk.response);

    //====================
    //===Parse Response===
    //====================
    health_check_response_t response = parse_response(chunk.response);

    printf("\n--------Health Check--------\n");
    printf("Status: %s\n", response.status);
    printf("active_sessions_count: %d\n", response.active_sessions_count);
    for (size_t i = 0; i < vector_size(&response.active_session_ids); i++) {
      char **id = (char **)vector_get(&response.active_session_ids, i);
      printf("ID (%zu): %s\n", i+1, *id);
    }
    printf("Timestamp: %s\n", response.timestamp);

    //=============
    //===Cleanup===
    //=============
    free(chunk.response);
    for (size_t i = 0; i < vector_size(&response.active_session_ids); ++i) {
      char **p_str = (char **)vector_get(&response.active_session_ids, i);
      free(*p_str);
    }
    vector_free(&response.active_session_ids);
    curl_global_cleanup();
  }
}
