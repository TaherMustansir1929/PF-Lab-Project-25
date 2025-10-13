#ifndef START_QUIZ_H
#define START_QUIZ_H

#include <cjson/cJSON.h>
#include "main.h"

#define START_QUIZ_URL "/api/quiz/mcqs"

typedef struct QuizMcqOptions {
  char *A;
  char *B;
  char *C;
  char*D;
} quiz_mcq_options_t;

typedef struct QuizStartResponse
{
  char *session_id;
  char *course;
  char *topic;
  char *question;
  quiz_mcq_options_t options;
  int question_number;
  int difficulty;
  char *message;

} quiz_start_response_t; 

void start_quiz(state_t* state);
quiz_start_response_t parse_quiz_start_response(const char *json_str);

#endif /* START_QUIZ_H */
