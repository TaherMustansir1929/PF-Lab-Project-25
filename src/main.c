#include "ansi-colors.h"
#include <answer-mcq.h>
#include <check-quiz-status.h>
#include <end-quiz.h>
#include <health-check.h>
#include <main.h>
#include <requests.h>
#include <start-quiz.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  int choice;
  int max_choice = 5;

  state_t state = {0};

  // Clear screen
  system("clear");

  while (1) {
    printf("\n" ANSI_FG_GREEN "---Welcome to FAST NUCES LMS---" ANSI_RESET
           "\n");
    printf("\n[1]. Generate MCQ\n[2]. Check Quiz Status\n[3]. End Quiz\n[4]. "
           "API Health Check\n[5]. Quit\n");
    printf("\n" ANSI_FG_CYAN "Select an option: " ANSI_RESET);
    scanf("%d", &choice);
    while (getchar() != '\n')
      ;

    if (choice < 1 || choice > max_choice) {
      fprintf(stderr,
              "\n" ANSI_FG_RED
              "Invalid option selected! Must be b/w [1-%d]" ANSI_RESET "\n",
              max_choice);
      continue;
    }

    switch (choice) {
    case 1:
      while (1) {
        start_quiz(&state);
        answer_mcq(state);

        printf(ANSI_DIM
               "Do you wish to generate another MCQ? (Y/N): " ANSI_RESET);
        while (getchar() != '\n')
          ;
        char opt;
        scanf(" %c", &opt);
        while (getchar() != '\n')
          ;
        if (opt != 'y' && opt != 'Y') {
          break;
        }
      }
      break;
    case 2:
      check_quiz_status(state.session_id);
      break;
    case 3:
      end_quiz(state.session_id);
      state.course = state.topic = NULL;
      break;
    case 4:
      health_check();
      break;
    case 5:
      printf(ANSI_FG_YELLOW
             "Are you sure you want to quit? (Y/N): " ANSI_RESET);
      char opt = getchar();
      if (opt == 'y' || opt == 'Y') {
        printf(
            ANSI_FG_CYAN
            "\nThank You for using our C program. Hope you learned something "
            "new today\n" ANSI_RESET);
        exit(0);
      } else {
        break;
      }
    default:
      printf("Invalid option!\n");
    }
  }

  return 0;
}
