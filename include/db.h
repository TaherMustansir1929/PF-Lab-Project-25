#ifndef DB_H
#define DB_H

#include <stdbool.h>

typedef struct QuizTable {
  int id;
  char username[255];
  char course[255];
  char topic[255];
  int total_questions;
  int score;
  char session_id[255];
} quiz_table_t;

bool db_init(const char *filename);
bool db_insert_quiz(const char* username, const char* course, const char* topic, const char* session_id);
bool db_get_quizes(const char* username);
bool db_update_score(int total_questions, int score, const char* username);

#endif // !DB_H
