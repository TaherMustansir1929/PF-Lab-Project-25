#ifndef CHECK_QUIZ_STATUS_H
#define CHECK_QUIZ_STATUS_H

#define CHECK_QUIZ_STATUS_URL "/api/quiz/status"

typedef struct CheckQuizStatusResponse {
  char *session_id;
  char *course;
  char *topic;
  int score;
  int total_questions;
  int difficulty;
  char *current_phase;
  char *created_at;
} check_quiz_status_response_t;

void check_quiz_status(const char *user_id, const char *session_id);

#endif // !CHECK_QUIZ_STATUS_H
