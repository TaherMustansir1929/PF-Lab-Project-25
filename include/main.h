#ifndef MAIN_H
#define MAIN_H

#include <cjson/cJSON.h>
#include <curl/curl.h>

#define HOST "host.docker.internal"
// #define BASE_URL "https://pf-lab-project-25-python-api.vercel.app"
#define BASE_URL "http://localhost:8000"

typedef struct State {
  char *session_id;
  char *user_id;
  char *course;
  char *topic;
  char *error;
} state_t;

// === GTK APPLICATION DEFINES ===

#define MAX_STUDENTS 100
#define ID_LENGTH 10
#define PASSWORD_LENGTH 50
#define MAX_MCQS 10
#define OPTION_LENGTH 200
#define QUESTION_LENGTH 300
#define COURSE_LENGTH 100
#define TOPIC_LENGTH 100
#define DETAILS_LENGTH 500

typedef struct Student {
  char student_id[ID_LENGTH];
  char password[PASSWORD_LENGTH];
} student_t;

typedef struct MCQ {
  char *question;
  char *option_a;
  char *option_b;
  char *option_c;
  char *option_d;
} mcq_t;

typedef struct Quiz {
  char *session_id;
  char *error;
  char course[COURSE_LENGTH];
  char topic[TOPIC_LENGTH];
  int current_question;
  int score;
} quiz_t;

typedef struct Profile {
  float cgpa;
  char major[DETAILS_LENGTH];
  char short_term_goals[DETAILS_LENGTH];
  char long_term_goals[DETAILS_LENGTH];
  char industries_of_interest[DETAILS_LENGTH];
  char target_roles[DETAILS_LENGTH];
} profile_t;

#endif
