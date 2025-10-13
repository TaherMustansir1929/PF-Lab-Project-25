#include "check-quiz-status.h"
#include "main.h"
#include "requests.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

check_quiz_status_response_t parse_quiz_status_response(const char* json_str) {
  check_quiz_status_response_t result = {0};

  cJSON *root = cJSON_Parse(json_str);
  if (!root) {
    perror("Failed to parse quiz_start_response\n");
    return result;
  }

  cJSON *session_id = cJSON_GetObjectItemCaseSensitive(root, "session_id");
  cJSON *course = cJSON_GetObjectItemCaseSensitive(root, "course");
  cJSON *topic = cJSON_GetObjectItemCaseSensitive(root, "topic");
  cJSON *score = cJSON_GetObjectItemCaseSensitive(root, "score");
  cJSON *total_questions = cJSON_GetObjectItemCaseSensitive(root, "total_questions");
  cJSON *difficulty = cJSON_GetObjectItemCaseSensitive(root, "difficulty");
  cJSON *current_phase = cJSON_GetObjectItemCaseSensitive(root, "current_phase");
  cJSON *created_at = cJSON_GetObjectItemCaseSensitive(root, "created_at");

  if(cJSON_IsString(session_id)) result.session_id = strdup(session_id->valuestring);
  if(cJSON_IsString(course)) result.course = strdup(course->valuestring);
  if(cJSON_IsString(topic)) result.topic = strdup(topic->valuestring);
  if(cJSON_IsNumber(score)) result.score = score->valueint;
  if(cJSON_IsNumber(total_questions)) result.total_questions = score->valueint;
  if(cJSON_IsNumber(difficulty)) result.difficulty = difficulty->valueint;
  if(cJSON_IsString(current_phase)) result.current_phase = strdup(current_phase->valuestring);
  if(cJSON_IsString(created_at)) result.created_at = strdup(created_at->valuestring);

  cJSON_Delete(root);
  return result;
}

void check_quiz_status(const char *session_id) {
  char url[255];
  snprintf(url, 255, "%s%s/%s", BASE_URL, CHECK_QUIZ_STATUS_URL, session_id);
  memory_t chunk = get_request(url);

  if (chunk.err != NULL) {
    fprintf(stderr, "Failed to recieve response: %s\n", chunk.err);
  } else {
    // DEBUG PRINT
    printf("\n\nRAW CHUNK RESPONSE: %s\n\n", chunk.response);

    //====================
    //===Parse Response===
    //====================
    check_quiz_status_response_t response = parse_quiz_status_response(chunk.response);

    printf("\n--------QUIZ STATUS--------\n");
    printf("session_id: %s\n", response.session_id);
    printf("course: %s\n", response.course);
    printf("topic: %s\n", response.topic);
    printf("difficulty: %d\n", response.difficulty);
    printf("score: %d\n", response.score);
    printf("total_questions: %d\n", response.total_questions);
    printf("current_phase: %s\n", response.current_phase);
    printf("created_at: %s\n",response.created_at);
  }

  free(chunk.response);
  curl_global_cleanup();
}
