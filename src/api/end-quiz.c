#include "end-quiz.h"
#include "ansi-colors.h"
#include "main.h"
#include "requests.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

end_quiz_response_t parse_end_quiz_response(const char *json_str) {
  end_quiz_response_t result = {0};
  cJSON *root = cJSON_Parse(json_str);
  if (!root) {
    perror("Failed to parse end_quiz response\n");
    return result;
  }

  cJSON *message = cJSON_GetObjectItemCaseSensitive(root, "message");
  if (cJSON_IsString(message))
    result.message = strdup(message->valuestring);

  cJSON *final_results =
      cJSON_GetObjectItemCaseSensitive(root, "final_results");
  if (cJSON_IsObject(final_results)) {
    cJSON *session_id =
        cJSON_GetObjectItemCaseSensitive(final_results, "session_id");
    cJSON *score = cJSON_GetObjectItemCaseSensitive(final_results, "score");
    cJSON *total_questions =
        cJSON_GetObjectItemCaseSensitive(final_results, "total_questions");
    cJSON *accuracy =
        cJSON_GetObjectItemCaseSensitive(final_results, "accuracy");
    cJSON *final_difficulty =
        cJSON_GetObjectItemCaseSensitive(final_results, "final_difficulty");

    if (cJSON_IsString(session_id))
      result.final_results.session_id = strdup(session_id->valuestring);
    if (cJSON_IsNumber(score))
      result.final_results.score = score->valueint;
    if (cJSON_IsNumber(total_questions))
      result.final_results.total_questions = total_questions->valueint;
    if (cJSON_IsNumber(accuracy))
      result.final_results.accuracy = accuracy->valueint;
    if (cJSON_IsNumber(final_difficulty))
      result.final_results.final_difficulty = final_difficulty->valueint;
  }

  cJSON_Delete(root);
  return result;
}

end_quiz_response_t end_quiz(const char *user_id, const char *session_id) {
  char url[255];
  const char *env = getenv("C_ENV");
  snprintf(url, sizeof(url), "%s%s/%s/%s",
           strcmp(env, "dev") ? BASE_URL_DEV : BASE_URL_PROD, END_QUIZ_URL,
           user_id, session_id);

  end_quiz_response_t response = {0};

  memory_t chunk = get_request(url);
  if (chunk.err != NULL) {
    fprintf(stderr, "Failed to recieve response: %s\n", chunk.err);
  } else {
    // DEBUG PRINT
    printf("\n\nRAW CHUNK RESPONSE: %s\n\n", chunk.response);

    //====================
    //===Parse Response===
    //====================
    response = parse_end_quiz_response(chunk.response);

    printf("\n--------QUIZ ENDED SUCCESSFULLY-------\n");
    printf("message: %s\n", response.message);
    printf("session_id: %s\n", response.final_results.session_id);
    printf("score: %d\n", response.final_results.score);
    printf("total_questions: %d\n", response.final_results.total_questions);
    printf((response.final_results.accuracy > 80)   ? ANSI_BG_GREEN
           : (response.final_results.accuracy > 50) ? ANSI_BG_YELLOW
                                                    : ANSI_BG_RED);
    printf("accuracy: %d" ANSI_RESET "\n", response.final_results.accuracy);
    printf("final_difficulty: %d\n", response.final_results.final_difficulty);
  }

  free(chunk.response);
  curl_global_cleanup();
  return response;
}
