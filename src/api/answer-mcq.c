#include "answer-mcq.h"
#include "ansi-colors.h"
#include "db.h"
#include "requests.h"
#include <cjson/cJSON.h>
#include <main.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

answer_mcq_response_t parse_answer_mcq_response(const char *json_str) {
  answer_mcq_response_t result = {0};
  cJSON *root = cJSON_Parse(json_str);
  if (!root) {
    perror("Failed to parse answer_mcq response\n");
    return result;
  }

  cJSON *session_id = cJSON_GetObjectItemCaseSensitive(root, "session_id");
  cJSON *feedback = cJSON_GetObjectItemCaseSensitive(root, "feedback");
  cJSON *is_correct = cJSON_GetObjectItemCaseSensitive(root, "is_correct");
  cJSON *score = cJSON_GetObjectItemCaseSensitive(root, "score");
  cJSON *total_questions =
      cJSON_GetObjectItemCaseSensitive(root, "total_questions");
  cJSON *new_difficulty =
      cJSON_GetObjectItemCaseSensitive(root, "new_difficulty");

  if (cJSON_IsString(session_id))
    result.session_id = strdup(session_id->valuestring);
  if (cJSON_IsString(feedback))
    result.feedback = strdup(feedback->valuestring);
  if (cJSON_IsBool(is_correct))
    result.is_correct = cJSON_IsTrue(is_correct);
  if (cJSON_IsNumber(score))
    result.score = score->valueint;
  if (cJSON_IsNumber(total_questions))
    result.total_questions = total_questions->valueint;
  if (cJSON_IsNumber(new_difficulty))
    result.new_difficulty = new_difficulty->valueint;

  cJSON_Delete(root);
  return result;
}

void answer_mcq(state_t state) {
  char answer[2];
  printf("\n" ANSI_BRIGHT_FG_CYAN "Enter you answer: " ANSI_RESET);
  snprintf(answer, sizeof(answer), "%c", getchar());

  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "session_id", state.session_id);
  cJSON_AddStringToObject(json, "user_id", state.user_id);
  cJSON_AddStringToObject(json, "answer", answer);

  char *json_data = cJSON_PrintUnformatted(json);
  if (!json_data) {
    fprintf(stderr, "Error creating json string\n");
    cJSON_Delete(json);
    return;
  }

  printf("\nJSON BEING SENT: %s", json_data);

  char url[255];
  snprintf(url, sizeof(url), "%s%s", BASE_URL, ANSWER_MCQ_URL);
  memory_t chunk = post_request(url, json_data);

  if (chunk.err != NULL) {
    fprintf(stderr, "Failed to recieve Response: %s", chunk.err);
  } else {
    // DEBUG PRINT
    printf("\n\nRAW CHUNK->RESPONSE: %s\n\n", chunk.response);
    //====================
    //===Parse Response===
    //====================
    answer_mcq_response_t response = parse_answer_mcq_response(chunk.response);

    printf("\n--------ANSWER FEEDBACK--------\n");
    printf("session_id: %s\n", response.session_id);
    printf(response.is_correct ? ANSI_BG_GREEN : ANSI_BG_YELLOW);
    printf("feedback: %s\n" ANSI_RESET, response.feedback);
    printf("is_correct: %s\n", response.is_correct ? "TRUE" : "FALSE");
    printf("score: %d\n", response.score);
    printf("total_questions: %d\n", response.total_questions);
    printf("new_difficulty: %d\n", response.new_difficulty);

    if (!db_update_score(response.total_questions, response.score,
                         state.session_id)) {
      fprintf(stderr, ANSI_FG_RED "FAILED to update score" ANSI_RESET "\n");
    }
  }

  //=============
  //===Cleanup===
  //=============
  free(chunk.response);
  free(json_data);
  cJSON_Delete(json);
  curl_global_cleanup();
  return;
}
