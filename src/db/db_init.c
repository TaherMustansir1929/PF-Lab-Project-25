#include "db.h"
#include <sqlite3.h>
#include <stdio.h>

static sqlite3 *db = NULL;

bool db_init(const char *filename) {
  if (sqlite3_open(filename, &db) != SQLITE_OK) {
    fprintf(stderr, "Failed to open database file: %s\n", sqlite3_errmsg(db));
    return false;
  }

  const char *sql = "CREATE TABLE IF NOT EXISTS quizes ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "session_id TEXT NOT NULL, "
                    "username TEXT NOT NULL, "
                    "course TEXT NOT NULL, "
                    "topic TEXT NOT NULL, "
                    "total_questions INTEGER NOT NULL, "
                    "score INTEGER NOT NULL);";

  char *errmsg = NULL;
  if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", errmsg);
    return false;
  }

  return true;
}

bool db_insert_quiz(const char *username, const char *course, const char *topic,
                    const char *session_id) {
  printf("username: %s\ncourse: %s\ntopic: %s\n", username, course, topic);
  const char *sql = "INSERT INTO quizes (username, course, topic, session_id, "
                    "total_questions, score) VALUES (?, ?, ?, ?, 0, 0);";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return false;
  }

  sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, course, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, topic, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 4, session_id, -1, SQLITE_TRANSIENT);

  bool success = sqlite3_step(stmt) == SQLITE_DONE;
  sqlite3_finalize(stmt);
  return success;
}

bool db_get_quizes(const char *username) {
  const char *sql =
      "SELECT id, username, course, topic, total_questions, session_id, "
      "score FROM quizes WHERE username = ?;";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement\n");
    return false;
  }

  sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

  int rc;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    quiz_table_t q;

    q.id = sqlite3_column_int(stmt, 0);
    snprintf(q.username, sizeof(q.username), "%s",
             sqlite3_column_text(stmt, 1));
    snprintf(q.course, sizeof(q.course), "%s", sqlite3_column_text(stmt, 2));
    snprintf(q.topic, sizeof(q.topic), "%s", sqlite3_column_text(stmt, 3));
    q.total_questions = sqlite3_column_int(stmt, 4);
    q.score = sqlite3_column_int(stmt, 5);
    snprintf(q.session_id, sizeof(q.session_id), "%s",
             sqlite3_column_text(stmt, 6));

    printf("-------------id: %d----------------\n", q.id);
    printf(
        "session_id: %s\nusername: %s\ncourse: %s\ntopic: %s\ntotal_questions: "
        "%d\nscore: %d\n",
        q.session_id, q.username, q.course, q.topic, q.total_questions,
        q.score);
  }

  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Error getting records: %s\n", sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE;
}

bool db_update_score(int total_questions, int score, const char *session_id) {
  printf("total_questions: %d, score: %d\n", total_questions, score);
  const char *sql =
      "UPDATE quizes SET total_questions = ?, score = ? WHERE session_id = ?";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return false;
  }

  sqlite3_bind_int(stmt, 1, total_questions);
  sqlite3_bind_int(stmt, 2, score);
  sqlite3_bind_text(stmt, 3, session_id, -1, SQLITE_TRANSIENT);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "Failed to update database: %s\n", sqlite3_errmsg(db));
    return false;
  }

  sqlite3_finalize(stmt);
  return true;
}
