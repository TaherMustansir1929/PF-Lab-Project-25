#ifndef END_QUIZ_H
#define END_QUIZ_H

#define END_QUIZ_URL "/api/quiz/end"

typedef struct FinalResults {
  char *session_id;
  int score;
  int total_questions;
  int accuracy;
  int final_difficulty;
} final_results_t;

typedef struct EndQuizResponse {
  char *message;
  final_results_t final_results;
} end_quiz_response_t;

void end_quiz(const char* session_id);

#endif // !END_QUIZ_H
