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
  vector_init(&result.quiz_session_ids, sizeof(user_sessions_t));

  cJSON *root = cJSON_Parse(json_str);
  if (!root) {
    perror("Failed to parse health_check_response\n");
    return result;
  }

  cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
  cJSON *active_user_count =
      cJSON_GetObjectItemCaseSensitive(root, "active_user_count");
  cJSON *quiz_session_count =
      cJSON_GetObjectItemCaseSensitive(root, "quiz_session_count");
  cJSON *quiz_session_ids =
      cJSON_GetObjectItemCaseSensitive(root, "quiz_session_ids");
  cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(root, "timestamp");

  if (cJSON_IsString(status))
    result.status = strdup(status->valuestring);
  if (cJSON_IsNumber(active_user_count))
    result.active_user_count = active_user_count->valueint;
  if (cJSON_IsNumber(quiz_session_count))
    result.quiz_session_count = quiz_session_count->valueint;
  if (cJSON_IsString(timestamp))
    result.timestamp = strdup(timestamp->valuestring);

  // Parse quiz_session_ids as a dictionary mapping user_id to list of
  // session_ids
  if (cJSON_IsObject(quiz_session_ids)) {
    cJSON *user_entry = NULL;
    cJSON_ArrayForEach(user_entry, quiz_session_ids) {
      if (cJSON_IsArray(user_entry)) {
        user_sessions_t user_sessions;
        user_sessions.user_id =
            strdup(user_entry->string); // Get the key (user_id)
        vector_init(&user_sessions.session_ids, sizeof(char *));

        // Iterate through the array of session IDs for this user
        cJSON *session_id = NULL;
        cJSON_ArrayForEach(session_id, user_entry) {
          if (cJSON_IsString(session_id) && (session_id->valuestring != NULL)) {
            char *session_copy = strdup(session_id->valuestring);
            if (session_copy == NULL) {
              fprintf(stderr, "strdup failed: out of memory\n");
              vector_free(&user_sessions.session_ids);
              free(user_sessions.user_id);
              continue;
            }
            vector_push_back(&user_sessions.session_ids, &session_copy);
          }
        }

        vector_push_back(&result.quiz_session_ids, &user_sessions);
      }
    }
  }

  cJSON_Delete(root);
  return result;
}

void health_check(void) {
  char url[256];
  snprintf(url, sizeof(url), "%s%s",
           BASE_URL_PROD, HEALTH_CHECK_URL);
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
    printf("Active User Count: %d\n", response.active_user_count);
    printf("Quiz Session Count: %d\n", response.quiz_session_count);
    printf("Quiz Sessions by User:\n");
    for (size_t i = 0; i < vector_size(&response.quiz_session_ids); i++) {
      user_sessions_t *user_sessions =
          (user_sessions_t *)vector_get(&response.quiz_session_ids, i);
      printf("  User: %s\n", user_sessions->user_id);
      for (size_t j = 0; j < vector_size(&user_sessions->session_ids); j++) {
        char **session_id = (char **)vector_get(&user_sessions->session_ids, j);
        printf("    Session ID (%zu): %s\n", j + 1, *session_id);
      }
    }
    printf("Timestamp: %s\n", response.timestamp);

    //=============
    //===Cleanup===
    //=============
    free(chunk.response);

    // Free all user sessions and their session IDs
    for (size_t i = 0; i < vector_size(&response.quiz_session_ids); ++i) {
      user_sessions_t *user_sessions =
          (user_sessions_t *)vector_get(&response.quiz_session_ids, i);

      // Free each session ID string
      for (size_t j = 0; j < vector_size(&user_sessions->session_ids); ++j) {
        char **p_str = (char **)vector_get(&user_sessions->session_ids, j);
        free(*p_str);
      }

      // Free the session_ids vector and user_id
      vector_free(&user_sessions->session_ids);
      free(user_sessions->user_id);
    }

    vector_free(&response.quiz_session_ids);
    free(response.status);
    free(response.timestamp);
    curl_global_cleanup();
  }
}
