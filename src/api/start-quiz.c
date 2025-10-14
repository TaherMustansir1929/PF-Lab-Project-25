#include "start-quiz.h"
#include "ansi-colors.h"
#include "db.h"
#include "requests.h"
#include <cjson/cJSON.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

quiz_start_response_t parse_quiz_start_response(const char *json_str) {
  quiz_start_response_t quiz = {0};
  cJSON *root = cJSON_Parse(json_str);
  if (!root) {
    perror("Failed to parse quiz_start_response\n");
    return quiz;
  }

  cJSON *session_id = cJSON_GetObjectItemCaseSensitive(root, "session_id");
  if (cJSON_IsString(session_id))
    quiz.session_id = strdup(session_id->valuestring);

  cJSON *course = cJSON_GetObjectItemCaseSensitive(root, "course");
  if (cJSON_IsString(course))
    quiz.course = strdup(course->valuestring);

  cJSON *topic = cJSON_GetObjectItemCaseSensitive(root, "topic");
  if (cJSON_IsString(topic))
    quiz.topic = strdup(topic->valuestring);

  cJSON *question = cJSON_GetObjectItemCaseSensitive(root, "question");
  if (cJSON_IsString(question))
    quiz.question = strdup(question->valuestring);

  {
    cJSON *options = cJSON_GetObjectItemCaseSensitive(root, "options");

    cJSON *A = cJSON_GetObjectItemCaseSensitive(options, "A");
    cJSON *B = cJSON_GetObjectItemCaseSensitive(options, "B");
    cJSON *C = cJSON_GetObjectItemCaseSensitive(options, "C");
    cJSON *D = cJSON_GetObjectItemCaseSensitive(options, "D");

    if (cJSON_IsString(A))
      quiz.options.A = strdup(A->valuestring);
    if (cJSON_IsString(B))
      quiz.options.B = strdup(B->valuestring);
    if (cJSON_IsString(C))
      quiz.options.C = strdup(C->valuestring);
    if (cJSON_IsString(D))
      quiz.options.D = strdup(D->valuestring);
  }

  cJSON *question_number =
      cJSON_GetObjectItemCaseSensitive(root, "question_number");
  if (cJSON_IsNumber(question_number))
    quiz.question_number = question_number->valueint;

  cJSON *difficulty = cJSON_GetObjectItemCaseSensitive(root, "difficulty");
  if (cJSON_IsNumber(difficulty))
    quiz.difficulty = difficulty->valueint;

  cJSON *message = cJSON_GetObjectItemCaseSensitive(root, "message");
  if (cJSON_IsString(message))
    quiz.message = strdup(message->valuestring);

  cJSON_Delete(root);
  return quiz;
}

void start_quiz(state_t *state) {
  char course[255], topic[255];
  if (state->course && state->topic && strlen(state->course) > 0 &&
      strlen(state->topic)) {
    snprintf(course, sizeof(course), "%s", state->course);
    snprintf(topic, sizeof(topic), "%s", state->topic);
  } else {
    //===========================
    //===Read course and topic===
    //===========================
    printf("Enter course: ");
    if (fgets(course, sizeof(course), stdin) == NULL) {
      fprintf(stderr, "Error reading course\n");
      return;
    }
    course[strcspn(course, "\n")] = '\0';
    printf("Enter topic: ");
    if (fgets(topic, sizeof(topic), stdin) == NULL) {
      fprintf(stderr, "Error reading topic\n");
      return;
    }
    topic[strcspn(topic, "\n")] = '\0';

    state->course = strdup(course);
    state->topic = strdup(topic);
  }
  //========================
  //===Create JSON object===
  //========================
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "course", course);
  cJSON_AddStringToObject(json, "topic", topic);
  cJSON_AddStringToObject(json, "user_id", state->username);
  // Only add session_id if it exists and is not empty
  if (state->session_id && strlen(state->session_id) > 0) {
    cJSON_AddStringToObject(json, "session_id", state->session_id);
  }
  cJSON_AddNumberToObject(json, "initial_difficulty", 2);

  char *json_data = cJSON_PrintUnformatted(json);
  if (!json_data) {
    fprintf(stderr, "Error creating json string\n");
    cJSON_Delete(json);
    return;
  }

  // DEBUG: Print the JSON being sent
  printf("\n\nJSON BEING SENT: %s\n\n", json_data);

  char url[256];
  snprintf(url, sizeof(url), "%s%s", BASE_URL, START_QUIZ_URL);
  memory_t chunk = post_request(url, json_data);

  if (chunk.err != NULL) {
    fprintf(stderr, "Failed to recieve Response: %s", chunk.err);
  } else {
    // DEBUG PRINT
    printf("\n\nRAW CHUNK->RESPONSE: %s\n\n", chunk.response);
    //====================
    //===Parse Response===
    //====================
    quiz_start_response_t response = parse_quiz_start_response(chunk.response);
    state->session_id = strdup(response.session_id);

    printf("\n--------Server Response---------\n");
    printf("session_id: %s\n", response.session_id);
    printf("course: %s\n", response.course);
    printf("topic: %s\n", response.topic);
    printf("question_number: %d\n", response.question_number);
    printf("difficulty: %d\n", response.difficulty);
    printf("message: %s\n", response.message);
    printf(ANSI_BG_GREEN "\nquestion: %s\n" ANSI_RESET, response.question);
    printf(ANSI_BG_BLUE "A) %s\n", response.options.A);
    printf("B) %s\n", response.options.B);
    printf("C) %s\n", response.options.C);
    printf("D) %s\n" ANSI_RESET, response.options.D);

    state->session_id = strdup(response.session_id);

    if (!db_insert_quiz(state->username, state->course, state->topic, state->session_id)) {
      fprintf(stderr, "! Failed to save data...\n");
    } else {
      printf("\n> Successfully added quiz to database...\n");
    }
  }

  //=============
  //===Cleanup===
  //=============
  free(chunk.response);
  free(json_data);
  cJSON_Delete(json);
  curl_global_cleanup();
}
