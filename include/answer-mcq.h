#ifndef ANSWER_MCQ_H
#define ANSWER_MCQ_H

#include <stdbool.h>

#define ANSWER_MCQ_URL "/api/quiz/answer"

typedef struct AnswerMcqResponse {
  char *session_id;
  bool is_correct;
  char *feedback;
  int score;
  int total_questions;
  int new_difficulty;
} answer_mcq_response_t;

answer_mcq_response_t answer_mcq(const char *session_id, const char *user_id,
                                 char ans);

#endif // !ANSWER_MCQ_H
