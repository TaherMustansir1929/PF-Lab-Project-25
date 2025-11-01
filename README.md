# PF Lab Project - GTK GUI Application

A comprehensive C-based desktop application with GTK3 GUI that provides an interactive quiz system and student profile analyzer. The application communicates with a custom REST API to deliver personalized educational experiences.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
3. [Building the Application](#building-the-application)
4. [Running the Application](#running-the-application)
5. [Application Features](#application-features)
6. [Project Structure](#project-structure)
7. [Technical Documentation](#technical-documentation)
8. [API Integration](#api-integration)
9. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Required System Packages

The application requires the following libraries and tools:

- **Build Tools:**
  - `clang` or `gcc` (C compiler)
  - `make` (build automation)
  - `pkg-config` (library configuration)

- **Required Libraries:**
  - `gtk+-3.0` (GTK3 GUI framework)
  - `libcurl` (HTTP client library)
  - `cJSON` (JSON parser)
  - `uuid` (UUID generation)
  - `sqlite3` (database support)

### Installation Commands

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y build-essential clang make pkg-config \
  libgtk-3-dev libcurl4-openssl-dev libcjson-dev uuid-dev libsqlite3-dev
```

#### Fedora/RHEL:
```bash
sudo dnf install -y gcc clang make pkg-config \
  gtk3-devel libcurl-devel cjson-devel libuuid-devel sqlite-devel
```

#### Arch Linux:
```bash
sudo pacman -S base-devel clang make pkg-config \
  gtk3 curl cjson util-linux-libs sqlite
```

---

## Installation

### Clone the Repository

```bash
git clone <repository-url>
cd "C app"
```

### Using Docker (Alternative)

A Dockerfile is provided for containerized builds:

```bash
docker build -t pf-lab-app .
docker run -it --rm -v $(pwd):/app pf-lab-app
```

---

## Building the Application

### Compilation

The project uses a Makefile for automated building:

```bash
make
```

This command:
- Compiles all `.c` files from `src/` and `src/api/` directories
- Links against required libraries (GTK3, libcurl, cJSON, UUID, SQLite3)
- Generates the `main` executable
- Includes debug symbols (`-g` flag)

### Makefile Targets

| Target | Description |
|--------|-------------|
| `make` or `make all` | Compile the entire project |
| `make run` | Build and immediately run the application |
| `make clean` | Remove the compiled executable |
| `make valgrind` | Run memory leak detection using Valgrind |

### Compiler Flags Explained

```makefile
CFLAGS = -Wall -Wextra -std=c11 -D_GNU_SOURCE -g $(shell pkg-config --cflags gtk+-3.0)
```

- `-Wall -Wextra`: Enable comprehensive warnings
- `-std=c11`: Use C11 standard
- `-D_GNU_SOURCE`: Enable GNU extensions
- `-g`: Include debugging symbols
- `$(shell pkg-config --cflags gtk+-3.0)`: Automatically include GTK3 headers

---

## Running the Application

### Basic Execution

```bash
./main
```

Or use the Makefile shortcut:

```bash
make run
```

### Environment Configuration

The application supports environment-based API configuration:

```bash
# Development mode (localhost:8000)
export C_ENV=dev
./main

# Production mode (Vercel deployment)
export C_ENV=prod
./main
```

**API Endpoints:**
- **Development:** `http://localhost:8000`
- **Production:** `https://pf-lab-project-25-python-api.vercel.app`

---

## Application Features

### 1. Student Authentication System
- **Sign In/Sign Up:** Create new accounts or log into existing ones
- **Student ID Format:** `##K-####` (e.g., `25K-0119`)
- **Password Protection:** Secure credential storage
- **Session Management:** Maintains login state during app usage

### 2. Quiz Application
- **Adaptive MCQ System:** Generates multiple-choice questions based on course and topic
- **Dynamic Difficulty:** Question difficulty adjusts based on performance
- **Real-time Feedback:** Immediate correctness validation with explanations
- **Session Tracking:** Maintains quiz progress across questions
- **Score Tracking:** Displays current score and accuracy percentage
- **Configurable Length:** Default 10 questions per quiz session

### 3. Profile Analyzer
- **Career Guidance:** AI-powered profile analysis and career recommendations
- **Input Parameters:**
  - CGPA (0.0 - 4.0 scale)
  - Major/Field of Study
  - Short-term Career Goals
  - Long-term Career Goals
  - Industries of Interest
  - Target Job Roles
- **Personalized Feedback:** Detailed career path recommendations
- **Export Functionality:** Save analysis results to markdown files

---

## Project Structure

```
C app/
├── Makefile                    # Build automation
├── compile_commands.json       # LSP/IDE configuration
├── Dockerfile                  # Container definition
├── main                        # Compiled executable (generated)
├── README.md                   # This file
│
├── include/                    # Header files
│   ├── ansi-colors.h          # Terminal color macros
│   ├── main.h                 # Core data structures and constants
│   ├── requests.h             # HTTP request function prototypes
│   ├── generate_mcq.h         # Quiz generation interface
│   ├── answer-mcq.h           # Answer submission interface
│   ├── end-quiz.h             # Quiz completion interface
│   ├── pf-analyzer.h          # Profile analyzer interface
│   ├── check-quiz-status.h    # Quiz status checking
│   ├── health-check.h         # API health check
│   ├── vector.h               # Dynamic array utilities
│   └── dotenv.h               # Environment variable loading
│
└── src/                        # Source files
    ├── app.c                  # GTK GUI application (main entry)
    ├── main.c                 # CLI version (legacy)
    ├── requests.c             # HTTP/CURL wrapper functions
    ├── vector.c               # Dynamic array implementation
    ├── dotenv.c               # Environment file parser
    │
    └── api/                   # API integration modules
        ├── generate_mcq.c     # Quiz generation implementation
        ├── answer-mcq.c       # Answer submission implementation
        ├── end-quiz.c         # Quiz completion implementation
        ├── pf-analyzer.c      # Profile analysis implementation
        ├── check-quiz-status.c # Quiz status implementation
        └── health-check.c     # Health check implementation
```

---

## Technical Documentation

### Core Data Structures

#### Student Structure (`student_t`)
```c
typedef struct Student {
  char student_id[ID_LENGTH];      // Format: ##K-####
  char password[PASSWORD_LENGTH];   // User password
} student_t;
```

#### Quiz Structure (`quiz_t`)
```c
typedef struct Quiz {
  char *session_id;               // Unique quiz session identifier
  char *error;                    // Error message (if any)
  char course[COURSE_LENGTH];     // Course name
  char topic[TOPIC_LENGTH];       // Topic name
  int current_question;           // Current question index (0-based)
  int score;                      // Current score
} quiz_t;
```

#### MCQ Structure (`mcq_t`)
```c
typedef struct MCQ {
  char *question;                 // Question text
  char *option_a;                // Option A text
  char *option_b;                // Option B text
  char *option_c;                // Option C text
  char *option_d;                // Option D text
} mcq_t;
```

#### Profile Structure (`profile_t`)
```c
typedef struct Profile {
  float cgpa;                                    // Student CGPA
  char major[DETAILS_LENGTH];                    // Major/Field
  char short_term_goals[DETAILS_LENGTH];         // Short-term goals
  char long_term_goals[DETAILS_LENGTH];          // Long-term goals
  char industries_of_interest[DETAILS_LENGTH];   // Target industries
  char target_roles[DETAILS_LENGTH];             // Target job roles
} profile_t;
```

---

### Module Breakdown

#### 1. **app.c** - GTK GUI Application (1077 lines)

**Purpose:** Main GUI application using GTK3 framework.

**Key Components:**

- **Global State Management:**
  ```c
  student_t students[MAX_STUDENTS];      // Student database (in-memory)
  student_t current_student;             // Currently logged-in student
  quiz_t current_quiz;                   // Active quiz session
  profile_t current_profile;             // Profile analysis data
  mcq_t mcqs[MAX_MCQS];                 // MCQ storage array
  ```

- **UI Widgets:**
  - `GtkStack *stack` - Multi-page navigation container
  - `GtkWidget *id_entry, *password_entry` - Authentication inputs
  - `GtkWidget *course_entry, *topic_entry` - Quiz setup inputs
  - `GtkWidget *question_label` - Question display
  - `GtkWidget *radio_a/b/c/d` - Answer selection radio buttons
  - Text views for profile analyzer inputs
  - Loading dialogs for async operations

**Key Functions:**

1. **`validate_student_id(const char *id)`**
   - Validates student ID format (##K-####)
   - Returns: `TRUE` if valid, `FALSE` otherwise

2. **`find_student(const char *id)`**
   - Searches student array for matching ID
   - Returns: Index if found, -1 if not found

3. **`add_student(const char *id, const char *password)`**
   - Adds new student to in-memory array
   - Manages MAX_STUDENTS capacity

4. **`on_signin_clicked(GtkWidget *widget, gpointer data)`**
   - Handles authentication logic
   - Validates credentials
   - Shows loading dialog with delayed processing
   - Switches to dashboard on success

5. **`on_start_quiz_clicked(GtkWidget *widget, gpointer data)`**
   - Validates course/topic inputs
   - Calls `generate_mcq()` API function
   - Initializes quiz state
   - Displays first question

6. **`on_submit_answer_clicked(GtkWidget *widget, gpointer data)`**
   - Captures selected radio button
   - Calls `answer_mcq()` API function
   - Displays feedback dialog
   - Advances to next question or ends quiz

7. **`on_end_quiz_clicked(GtkWidget *widget, gpointer data)`**
   - Confirms quiz termination
   - Calls `end_quiz()` API function
   - Displays final results
   - Resets quiz state

8. **`on_submit_profile_clicked(GtkWidget *widget, gpointer data)`**
   - Validates profile inputs (CGPA range, field completeness)
   - Calls `pf_analyzer()` API function
   - Displays feedback in scrollable text view
   - Option to save results to file

9. **`reset_quiz_state()`**
   - Frees all dynamically allocated strings
   - Resets quiz and MCQ structures
   - Prevents memory leaks between sessions

10. **`activate(GtkApplication *app, gpointer user_data)`**
    - Builds entire GTK UI hierarchy
    - Creates all pages (signup, dashboard, quiz, profile analyzer)
    - Connects signal handlers
    - Shows main window

**GTK Application Flow:**
```
activate()
    ↓
Create GtkStack (multi-page container)
    ↓
Build Pages:
    1. Sign Up/Sign In Page
    2. Dashboard Page
    3. Quiz Application Page
        - Quiz Setup Box (course/topic entry)
        - Quiz Content Box (questions/answers)
    4. Profile Analyzer Page
        - Profile Form Box (input fields)
        - Profile Feedback Box (results display)
    ↓
Connect Signal Handlers (button clicks)
    ↓
Show Main Window
```

---

#### 2. **requests.c** - HTTP Client Module

**Purpose:** Abstraction layer for HTTP requests using libcurl.

**Key Structures:**

```c
typedef struct Memory {
  char *response;    // Response body
  size_t size;       // Response size
  char *err;         // Error message
} memory_t;
```

**Functions:**

1. **`write_callback(void *contents, size_t size, size_t nmemb, void *userp)`**
   - Callback invoked by libcurl when receiving data
   - Dynamically reallocates buffer as data arrives
   - Appends new data to existing buffer
   - Returns total bytes written

2. **`post_request(const char *url, const char *json_data)`**
   - Sends HTTP POST request with JSON payload
   - Sets `Content-Type: application/json` header
   - Returns `memory_t` with response or error
   - Handles CURL initialization and cleanup

3. **`get_request(const char *url)`**
   - Sends HTTP GET request
   - Returns `memory_t` with response or error
   - Enables verbose output for debugging

**Usage Pattern:**
```c
memory_t chunk = post_request(url, json_data);
if (chunk.err != NULL) {
  // Handle error
} else {
  // Process chunk.response
}
free(chunk.response);
curl_global_cleanup();
```

---

#### 3. **generate_mcq.c** - Quiz Generation Module

**Purpose:** Starts quiz sessions and retrieves questions from API.

**API Endpoint:** `POST /api/quiz/mcqs`

**Request Payload:**
```json
{
  "course": "Computer Science",
  "topic": "Data Structures",
  "user_id": "25K-0119",
  "session_id": "uuid-string",  // Optional, for continuation
  "initial_difficulty": 2
}
```

**Response Structure:**
```c
typedef struct QuizStartResponse {
  char *session_id;           // Unique session UUID
  char *course;               // Course name
  char *topic;                // Topic name
  char *question;             // Question text
  quiz_mcq_options_t options; // A, B, C, D options
  int question_number;        // Question index
  int difficulty;             // Current difficulty (1-5)
  char *message;              // Status/error message
} quiz_start_response_t;
```

**Functions:**

1. **`parse_quiz_start_response(const char *json_str)`**
   - Parses JSON response using cJSON library
   - Handles error responses (checks for "detail" field)
   - Duplicates all strings for safe memory management
   - Returns populated `quiz_start_response_t` structure

2. **`generate_mcq(quiz_t *state, const char *student_id)`**
   - Creates JSON request payload
   - Selects appropriate URL (dev/prod) based on `C_ENV`
   - Sends POST request
   - Parses response
   - Updates quiz state with session_id
   - Prints formatted question to console
   - Returns response structure

**Error Handling:**
- Checks for NULL session_id (indicates failure)
- Sets `state->error` on failure
- Clears error on success

---

#### 4. **answer-mcq.c** - Answer Submission Module

**Purpose:** Submits student answers and receives feedback.

**API Endpoint:** `POST /api/quiz/answer`

**Request Payload:**
```json
{
  "session_id": "uuid-string",
  "user_id": "25K-0119",
  "answer": "B"
}
```

**Response Structure:**
```c
typedef struct AnswerMcqResponse {
  char *session_id;       // Session identifier
  bool is_correct;        // Answer correctness
  char *feedback;         // Explanation text
  int score;              // Current score
  int total_questions;    // Questions answered
  int new_difficulty;     // Updated difficulty level
} answer_mcq_response_t;
```

**Functions:**

1. **`parse_answer_mcq_response(const char *json_str)`**
   - Parses JSON response
   - Handles boolean `is_correct` field
   - Returns populated structure

2. **`answer_mcq(const char *session_id, const char *user_id, char ans)`**
   - Converts char answer to string format
   - Sends POST request
   - Parses feedback
   - Prints colored feedback (green=correct, yellow=incorrect)
   - Returns response structure

---

#### 5. **end-quiz.c** - Quiz Completion Module

**Purpose:** Retrieves final quiz results and statistics.

**API Endpoint:** `GET /api/quiz/end/{user_id}/{session_id}`

**Response Structure:**
```c
typedef struct FinalResults {
  char *session_id;        // Session identifier
  int score;               // Final score
  int total_questions;     // Total answered
  int accuracy;            // Percentage (0-100)
  int final_difficulty;    // Ending difficulty
} final_results_t;

typedef struct EndQuizResponse {
  char *message;           // Success message
  final_results_t final_results;
} end_quiz_response_t;
```

**Functions:**

1. **`parse_end_quiz_response(const char *json_str)`**
   - Parses nested JSON structure
   - Extracts final_results object
   - Returns populated response

2. **`end_quiz(const char *user_id, const char *session_id)`**
   - Constructs GET URL with user_id and session_id
   - Sends request
   - Parses results
   - Prints colored accuracy (green>80%, yellow>50%, red<=50%)
   - Returns final results

---

#### 6. **pf-analyzer.c** - Profile Analysis Module

**Purpose:** Sends student profile data for AI-powered career analysis.

**API Endpoint:** `POST /api/pfanalyzer`

**Request Structure:**
```c
typedef struct PFAnalyzerRequest {
  char *student_id;
  float cgpa;
  char *major;
  char *short_term_goals;
  char *long_term_goals;
  char *industries_of_interest;
  char *target_roles;
} pf_analyzer_request_t;
```

**Response Structure:**
```c
typedef struct PFAnalyzerResponse {
  char *feedback;    // AI-generated career advice
  char *timestamp;   // Analysis timestamp
  char *message;     // Error/status message
} pf_analyzer_response_t;
```

**Functions:**

1. **`parse_pf_analyzer_response(const char *json_str)`**
   - Parses API response
   - Handles error responses with "detail" field
   - Returns feedback and timestamp

2. **`pf_analyzer(const pf_analyzer_request_t *request)`**
   - Validates input parameters
   - Checks CGPA range (0.0 - 10.0)
   - Creates JSON payload
   - Sends POST request
   - Parses and displays feedback
   - Returns response structure

3. **`save_pf_analyzer_response_to_file(const pf_analyzer_response_t *response, const char *student_id)`**
   - Creates `profile_analysis/` directory
   - Generates filename: `{student_id}_{timestamp}.md`
   - Sanitizes timestamp for filesystem
   - Writes feedback in Markdown format
   - Returns 0 on success, -1 on failure

4. **`free_pf_analyzer_response(pf_analyzer_response_t *response)`**
   - Frees dynamically allocated strings
   - Prevents memory leaks

---

### Memory Management

**Critical Practices:**

1. **String Duplication:**
   - All strings from cJSON are duplicated using `strdup()`
   - Ensures data persists after JSON object deletion

2. **Cleanup Sequence:**
   ```c
   free(chunk.response);        // Free HTTP response
   free(json_data);             // Free JSON string
   cJSON_Delete(json);          // Free cJSON object
   curl_global_cleanup();       // Cleanup libcurl
   ```

3. **Quiz State Reset:**
   - `reset_quiz_state()` frees all dynamic allocations
   - Called before starting new quiz
   - Prevents memory accumulation

4. **GTK Widget Management:**
   - GTK handles widget memory automatically
   - No manual `free()` needed for widgets
   - Use `gtk_widget_destroy()` for dialogs

---

### API Integration

**Base URLs:**
```c
#define BASE_URL_DEV "http://localhost:8000"
#define BASE_URL_PROD "https://pf-lab-project-25-python-api.vercel.app"
```

**Endpoint Selection:**
```c
const char *env = getenv("C_ENV");
snprintf(url, sizeof(url), "%s%s",
         strcmp(env, "dev") == 0 ? BASE_URL_DEV : BASE_URL_PROD,
         ENDPOINT_PATH);
```

**Available Endpoints:**

| Endpoint | Method | Purpose |
|----------|--------|---------|
| `/api/quiz/mcqs` | POST | Generate/retrieve MCQ |
| `/api/quiz/answer` | POST | Submit answer |
| `/api/quiz/end/{user_id}/{session_id}` | GET | End quiz session |
| `/api/pfanalyzer` | POST | Analyze student profile |

---

### Constants and Configuration

```c
// Limits
#define MAX_STUDENTS 100          // Maximum student capacity
#define MAX_MCQS 10              // Questions per quiz session
#define ID_LENGTH 10             // Student ID buffer size
#define PASSWORD_LENGTH 50       // Password buffer size
#define COURSE_LENGTH 100        // Course name buffer
#define TOPIC_LENGTH 100         // Topic name buffer
#define DETAILS_LENGTH 500       // Profile detail buffer
#define OPTION_LENGTH 200        // MCQ option buffer
#define QUESTION_LENGTH 300      // Question text buffer
```

---

## Troubleshooting

### Build Errors

**Problem:** `pkg-config: command not found`
```bash
sudo apt-get install pkg-config
```

**Problem:** `gtk/gtk.h: No such file or directory`
```bash
sudo apt-get install libgtk-3-dev
```

**Problem:** `curl/curl.h: No such file or directory`
```bash
sudo apt-get install libcurl4-openssl-dev
```

### Runtime Errors

**Problem:** API connection fails
- Check `C_ENV` environment variable
- Verify API server is running (for dev mode)
- Check network connectivity

**Problem:** Segmentation fault
- Run with Valgrind: `make valgrind`
- Check for uninitialized pointers
- Verify all API responses are valid

**Problem:** Memory leaks
```bash
valgrind --leak-check=full --show-leak-kinds=all ./main
```

### GTK Display Issues

**Problem:** Application doesn't launch
- Verify X11/Wayland display server is running
- Check `DISPLAY` environment variable
- Run: `echo $DISPLAY` (should show `:0` or similar)

---

## Future Enhancements

- [ ] Persistent storage using SQLite database
- [ ] Session recovery after application restart
- [ ] Multi-user quiz leaderboards
- [ ] Export quiz results to PDF
- [ ] Offline mode with cached questions
- [ ] Custom quiz difficulty settings
- [ ] Timed quiz mode
- [ ] Quiz history and analytics

---

## License

[Specify your license here]

## Contributors

[List contributors here]

---

**Last Updated:** November 2025
