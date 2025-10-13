# PF Lab Project C App

## Build and Run

- `make` — compile the project into the `main` executable.
- `make run` — rebuild if needed and launch the interactive CLI.
- `make clean` — remove the compiled binary.

## Project Overview

C-based terminal application that talks to a custom quiz API. It generates MCQs for a selected course and topic, lets the user answer them, checks quiz progress, and will eventually gain a GUI front end.

## Directory Guide

- `Makefile` — defines build, run, clean, and Valgrind targets.
- `compile_commands.json` — compilation database for IDE tooling.
- `Dockerfile` — container recipe for running the app in a consistent environment.
- `main` — compiled executable produced by the build.

### include/

- `ansi-colors.h` — ANSI escape sequences for colored terminal output.
- `main.h` — shared types such as `state_t` and core API constants.
- `requests.h` — prototypes for the HTTP helper functions.
- `vector.h` — generic dynamic array API used to store variable-length data.
- `start-quiz.h` — request/response structs plus the `start_quiz` entry point.
- `answer-mcq.h` — structures for answer submission and the `answer_mcq` helper.
- `check-quiz-status.h` — response model and interface to poll quiz status.
- `end-quiz.h` — data model for final results and the `end_quiz` helper.
- `health-check.h` — types and function for the API health endpoint.

### src/

- `main.c` — interactive CLI loop orchestrating the quiz workflow.
- `requests.c` — libcurl wrappers that perform HTTP POST/GET calls.
- `vector.c` — implementation of the reusable dynamic vector.

#### src/api/

- `start-quiz.c` — gathers course/topic input, starts a quiz, and prints the MCQ.
- `answer-mcq.c` — submits answers, parses feedback, and displays scoring info.
- `check-quiz-status.c` — fetches current quiz progress and session metadata.
- `end-quiz.c` — retrieves final results and prints a completion summary.
- `health-check.c` — calls the health endpoint and lists active sessions.

## Future Direction

- Transition from CLI to a GTK-based GUI while reusing the API layer.
- Harden error handling, add persistent session storage, and expand quiz modes.
