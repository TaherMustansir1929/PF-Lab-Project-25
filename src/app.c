#include "end-quiz.h"
#include "gio/gio.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ansi-colors.h"
#include "answer-mcq.h"
#include "end-quiz.h"
#include "generate_mcq.h"
#include "main.h"
#include "pf-analyzer.h"

// Function to convert Markdown to Pango markup
char* markdown_to_pango(const char* markdown) {
  if (!markdown) return NULL;
  
  size_t len = strlen(markdown);
  // Allocate generous buffer (3x original size for markup tags)
  char* result = malloc(len * 3 + 1);
  if (!result) return NULL;
  
  const char* src = markdown;
  char* dst = result;
  
  while (*src) {
    // Handle headings (# Header)
    if (*src == '#' && (src == markdown || *(src - 1) == '\n')) {
      int level = 0;
      while (*src == '#' && level < 6) {
        level++;
        src++;
      }
      while (*src == ' ') src++; // Skip spaces after #
      
      // Add bold and larger text for headings
      const char* size_tags[] = {"xx-large", "x-large", "large", "large", "medium", "medium"};
      int size_idx = (level - 1 < 6) ? level - 1 : 5;
      dst += sprintf(dst, "<span size='%s' weight='bold'>", size_tags[size_idx]);
      
      // Copy until end of line
      while (*src && *src != '\n') {
        if (*src == '<') dst += sprintf(dst, "&lt;");
        else if (*src == '>') dst += sprintf(dst, "&gt;");
        else if (*src == '&') dst += sprintf(dst, "&amp;");
        else *dst++ = *src;
        src++;
      }
      dst += sprintf(dst, "</span>");
      if (*src == '\n') {
        *dst++ = '\n';
        src++;
      }
      continue;
    }
    
    // Handle bold (**text** or __text__)
    if ((*src == '*' && *(src + 1) == '*') || (*src == '_' && *(src + 1) == '_')) {
      char marker = *src;
      src += 2;
      dst += sprintf(dst, "<b>");
      while (*src && !(*src == marker && *(src + 1) == marker)) {
        if (*src == '<') dst += sprintf(dst, "&lt;");
        else if (*src == '>') dst += sprintf(dst, "&gt;");
        else if (*src == '&') dst += sprintf(dst, "&amp;");
        else *dst++ = *src;
        src++;
      }
      dst += sprintf(dst, "</b>");
      if (*src == marker && *(src + 1) == marker) src += 2;
      continue;
    }
    
    // Handle italic (*text* or _text_)
    if (*src == '*' || *src == '_') {
      char marker = *src;
      src++;
      dst += sprintf(dst, "<i>");
      while (*src && *src != marker) {
        if (*src == '<') dst += sprintf(dst, "&lt;");
        else if (*src == '>') dst += sprintf(dst, "&gt;");
        else if (*src == '&') dst += sprintf(dst, "&amp;");
        else *dst++ = *src;
        src++;
      }
      dst += sprintf(dst, "</i>");
      if (*src == marker) src++;
      continue;
    }
    
    // Handle bullet points (- item or * item)
    if ((*src == '-' || *src == '*') && (src == markdown || *(src - 1) == '\n') && *(src + 1) == ' ') {
      src += 2; // Skip marker and space
      dst += sprintf(dst, "  • ");
      continue;
    }
    
    // Handle numbered lists (1. item)
    if (isdigit(*src) && (src == markdown || *(src - 1) == '\n')) {
      const char* num_start = src;
      while (isdigit(*src)) src++;
      if (*src == '.' && *(src + 1) == ' ') {
        // Copy the number
        while (num_start < src) *dst++ = *num_start++;
        *dst++ = '.';
        src += 2; // Skip '. '
        continue;
      } else {
        // Not a list, reset and copy normally
        src = num_start;
      }
    }
    
    // Escape special XML characters
    if (*src == '<') {
      dst += sprintf(dst, "&lt;");
      src++;
    } else if (*src == '>') {
      dst += sprintf(dst, "&gt;");
      src++;
    } else if (*src == '&') {
      dst += sprintf(dst, "&amp;");
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  
  *dst = '\0';
  return result;
}

// Global variables
student_t students[MAX_STUDENTS];
int student_count = 0;
student_t current_student;
quiz_t current_quiz;
profile_t current_profile;
int mcq_count = 0;

// Hardcoded MCQ array
mcq_t mcqs[MAX_MCQS] = {0};

GtkWidget *stack;
GtkWidget *id_entry;
GtkWidget *password_entry;
GtkWidget *info_label;

// Quiz widgets
GtkWidget *course_entry;
GtkWidget *topic_entry;
GtkWidget *quiz_setup_box;
GtkWidget *quiz_content_box;
GtkWidget *question_label;
GtkWidget *radio_a, *radio_b, *radio_c, *radio_d;

// Profile analyzer widgets
GtkWidget *cgpa_entry;
GtkWidget *major_entry;
GtkWidget *short_term_goals_text;
GtkWidget *long_term_goals_text;
GtkWidget *industries_of_interest_text;
GtkWidget *target_roles_text;
GtkWidget *profile_form_box;
GtkWidget *profile_feedback_box;
GtkWidget *feedback_label;

// Loading widgets
GtkWidget *loading_dialog;

// Function to validate student ID format (e.g., 25K-0119)
gboolean validate_student_id(const char *id) {
  int year, num;
  // Format: ##K-#### (two digits, K, dash, four digits)
  if (sscanf(id, "%2dK-%4d", &year, &num) == 2) {
    // Verify the 'K' is in the right position
    if (strlen(id) == 8 && id[2] == 'K' && id[3] == '-') {
      return TRUE;
    }
  }
  return FALSE;
}

// Function to find student in array
int find_student(const char *id) {
  for (int i = 0; i < student_count; i++) {
    if (strcmp(students[i].student_id, id) == 0) {
      return i;
    }
  }
  return -1;
}

// Function to add new student
void add_student(const char *id, const char *password) {
  if (student_count < MAX_STUDENTS) {
    strncpy(students[student_count].student_id, id, ID_LENGTH - 1);
    students[student_count].student_id[ID_LENGTH - 1] = '\0';
    strncpy(students[student_count].password, password, PASSWORD_LENGTH - 1);
    students[student_count].password[PASSWORD_LENGTH - 1] = '\0';
    student_count++;
  }
}

// Show loading dialog
void show_loading_dialog(const char *message) {
  loading_dialog =
      gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                             GTK_BUTTONS_NONE, "%s", message);
  gtk_window_set_title(GTK_WINDOW(loading_dialog), "Loading");
  gtk_widget_show_all(loading_dialog);

  // Process events to show the dialog
  while (gtk_events_pending()) {
    gtk_main_iteration();
  }
}

// Close loading dialog
void close_loading_dialog() {
  if (loading_dialog) {
    gtk_widget_destroy(loading_dialog);
    loading_dialog = NULL;
  }
}

// Callback data structures for delayed operations
typedef struct {
  char id[ID_LENGTH];
  char password[PASSWORD_LENGTH];
  int is_new_student;
} SignInData;

// Delayed sign-in processing
gboolean process_signin_delayed(gpointer user_data) {
  SignInData *data = (SignInData *)user_data;

  close_loading_dialog();

  // Update info label
  char info_text[200];
  if (data->is_new_student) {
    snprintf(info_text, sizeof(info_text),
             "Welcome!\n\nStudent ID: %s\nPassword: %s\n\n"
             "Status: New Student Registered",
             data->id, data->password);
  } else {
    snprintf(info_text, sizeof(info_text),
             "Welcome Back!\n\nStudent ID: %s\nPassword: %s\n\n"
             "Status: Existing Student",
             data->id, data->password);
  }
  gtk_label_set_text(GTK_LABEL(info_label), info_text);

  // Switch to dashboard
  gtk_stack_set_visible_child_name(GTK_STACK(stack), "dashboard");

  g_free(data);
  return FALSE;
}

// Callback for sign-in button
void on_signin_clicked(GtkWidget *widget, gpointer data) {
  const char *id = gtk_entry_get_text(GTK_ENTRY(id_entry));
  const char *password = gtk_entry_get_text(GTK_ENTRY(password_entry));

  // Validate ID format
  if (!validate_student_id(id)) {
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        "Invalid Student ID format!\nExpected format: ##K-#### (e.g., "
        "25K-0119)");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return;
  }

  // Check for empty password
  if (strlen(password) == 0) {
    GtkWidget *dialog =
        gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                               GTK_BUTTONS_OK, "Password cannot be empty!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return;
  }

  // Search for student
  int index = find_student(id);

  if (index != -1) {
    // Student found - verify password
    if (strcmp(students[index].password, password) == 0) {
      // Copy to current student
      strcpy(current_student.student_id, students[index].student_id);
      strcpy(current_student.password, students[index].password);

      // Show loading
      show_loading_dialog("Signing in...\nPlease wait...");

      // Prepare callback data
      SignInData *signin_data = g_new(SignInData, 1);
      strcpy(signin_data->id, current_student.student_id);
      strcpy(signin_data->password, current_student.password);
      signin_data->is_new_student = 0;

      // Clear entries
      gtk_entry_set_text(GTK_ENTRY(id_entry), "");
      gtk_entry_set_text(GTK_ENTRY(password_entry), "");

      // Schedule delayed processing (1.5 seconds)
      g_timeout_add(1500, process_signin_delayed, signin_data);
    } else {
      GtkWidget *dialog =
          gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                 GTK_BUTTONS_OK, "Incorrect password!");
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(dialog);
    }
  } else {
    // New student - add to array
    add_student(id, password);
    strcpy(current_student.student_id, id);
    strcpy(current_student.password, password);

    // Show loading
    show_loading_dialog("Creating new account...\nPlease wait...");

    // Prepare callback data
    SignInData *signin_data = g_new(SignInData, 1);
    strcpy(signin_data->id, current_student.student_id);
    strcpy(signin_data->password, current_student.password);
    signin_data->is_new_student = 1;

    // Clear entries
    gtk_entry_set_text(GTK_ENTRY(id_entry), "");
    gtk_entry_set_text(GTK_ENTRY(password_entry), "");

    // Schedule delayed processing (1.5 seconds)
    g_timeout_add(1500, process_signin_delayed, signin_data);
  }
}

// Callbacks for page navigation
void on_page1_clicked(GtkWidget *widget, gpointer data) {
  gtk_stack_set_visible_child_name(GTK_STACK(stack), "page1");
}

void on_page2_clicked(GtkWidget *widget, gpointer data) {
  gtk_stack_set_visible_child_name(GTK_STACK(stack), "page2");
}

void on_back_clicked(GtkWidget *widget, gpointer data) {
  gtk_stack_set_visible_child_name(GTK_STACK(stack), "dashboard");
}

void on_logout_clicked(GtkWidget *widget, gpointer data) {
  memset(&current_student, 0, sizeof(student_t));
  gtk_stack_set_visible_child_name(GTK_STACK(stack), "signup");
}

// Quiz functions

// Helper function to reset all quiz state
void reset_quiz_state() {
  // Free dynamically allocated session_id if it exists
  if (current_quiz.session_id) {
    free(current_quiz.session_id);
    current_quiz.session_id = NULL;
  }

  // Free dynamically allocated error if it exists
  if (current_quiz.error) {
    free(current_quiz.error);
    current_quiz.error = NULL;
  }

  // Free all MCQ question and option strings
  for (int i = 0; i < MAX_MCQS; i++) {
    if (mcqs[i].question) {
      free(mcqs[i].question);
      mcqs[i].question = NULL;
    }
    if (mcqs[i].option_a) {
      free(mcqs[i].option_a);
      mcqs[i].option_a = NULL;
    }
    if (mcqs[i].option_b) {
      free(mcqs[i].option_b);
      mcqs[i].option_b = NULL;
    }
    if (mcqs[i].option_c) {
      free(mcqs[i].option_c);
      mcqs[i].option_c = NULL;
    }
    if (mcqs[i].option_d) {
      free(mcqs[i].option_d);
      mcqs[i].option_d = NULL;
    }
  }

  // Reset the quiz struct
  memset(&current_quiz, 0, sizeof(quiz_t));

  // Reset MCQ array
  memset(mcqs, 0, sizeof(mcqs));

  // Reset MCQ counter
  mcq_count = 0;
}
void display_question() {
  if (current_quiz.current_question >= MAX_MCQS) {
    return;
  }

  mcq_t *mcq = &mcqs[mcq_count];

  char question_text[500];
  snprintf(question_text, sizeof(question_text),
           "Course: %s | Topic: %s | Question %d of %d\n\n<b>%s</b>",
           current_quiz.course, current_quiz.topic,
           current_quiz.current_question + 1, MAX_MCQS, mcq->question);
  gtk_label_set_markup(GTK_LABEL(question_label), question_text);

  gtk_button_set_label(GTK_BUTTON(radio_a), mcq->option_a);
  gtk_button_set_label(GTK_BUTTON(radio_b), mcq->option_b);
  gtk_button_set_label(GTK_BUTTON(radio_c), mcq->option_c);
  gtk_button_set_label(GTK_BUTTON(radio_d), mcq->option_d);

  // Reset radio buttons
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_a), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_b), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_c), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_d), FALSE);
}

// Delayed quiz start
gboolean start_quiz_delayed(gpointer user_data) {
  close_loading_dialog();

  // Show quiz content
  gtk_widget_hide(quiz_setup_box);
  gtk_widget_show_all(quiz_content_box);

  display_question();

  return FALSE;
}

void on_start_quiz_clicked(GtkWidget *widget, gpointer data) {
  (void)widget; // Suppress unused parameter warning
  (void)data;   // Suppress unused parameter warning

  const char *course = gtk_entry_get_text(GTK_ENTRY(course_entry));
  const char *topic = gtk_entry_get_text(GTK_ENTRY(topic_entry));

  if (strlen(course) == 0 || strlen(topic) == 0) {
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        "Please enter both course and topic!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return;
  }

  // Reset quiz state from any previous session
  reset_quiz_state();

  // Initialize quiz with new course and topic
  strncpy(current_quiz.course, course, COURSE_LENGTH - 1);
  current_quiz.course[COURSE_LENGTH - 1] = '\0';
  strncpy(current_quiz.topic, topic, TOPIC_LENGTH - 1);
  current_quiz.topic[TOPIC_LENGTH - 1] = '\0';

  // Show loading
  show_loading_dialog("Loading quiz...\nPreparing questions...");

  quiz_start_response_t response =
      generate_mcq(&current_quiz, current_student.student_id);

  // Check if quiz start was successful
  if (response.session_id == NULL) {
    close_loading_dialog();
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        "Failed to start quiz!\n\nServer Error: %s",
        response.message ? response.message : "Unknown error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return;
  }

  current_quiz.session_id = response.session_id;
  current_quiz.current_question = response.question_number;

  mcq_t current_mcq = {response.question, response.options.A,
                       response.options.B, response.options.C,
                       response.options.D};

  mcqs[mcq_count] = current_mcq;

  // Schedule delayed quiz start (2 seconds)
  start_quiz_delayed(NULL);
}

void on_submit_answer_clicked(GtkWidget *widget, gpointer data) {
  char selected_option = '\0';

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_a))) {
    selected_option = 'A';
  } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_b))) {
    selected_option = 'B';
  } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_c))) {
    selected_option = 'C';
  } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_d))) {
    selected_option = 'D';
  }

  if (selected_option == '\0') {
    GtkWidget *dialog =
        gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
                               GTK_BUTTONS_OK, "Please select an option!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return;
  }

  // Check answer
  answer_mcq_response_t response = answer_mcq(
      current_quiz.session_id, current_student.student_id, selected_option);
  gboolean is_correct = (response.is_correct);

  // Show feedback - with NULL check for response.feedback
  char feedback[400];
  const char *feedback_text =
      response.feedback ? response.feedback : "No feedback available";

  if (is_correct) {
    snprintf(feedback, sizeof(feedback),
             "✓ Correct!\n\nYour answer: %c\nScore: %d/%d\nFeedback: %s",
             selected_option, response.score, response.total_questions,
             feedback_text);
  } else {
    snprintf(feedback, sizeof(feedback),
             "✗ Incorrect!\n\nYour answer: %c\nScore: "
             "%d/%d\nFeedback: %s",
             selected_option, response.score, response.total_questions,
             feedback_text);
  }

  GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                                             is_correct ? GTK_MESSAGE_INFO
                                                        : GTK_MESSAGE_WARNING,
                                             GTK_BUTTONS_OK, "%s", feedback);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  // Increment question counter
  current_quiz.current_question++;

  // Check if quiz is finished (reached MAX_MCQS questions)
  if (current_quiz.current_question >= MAX_MCQS) {
    end_quiz_response_t end_response =
        end_quiz(current_student.student_id, current_quiz.session_id);
    // Quiz finished
    char final_msg[400];
    snprintf(final_msg, sizeof(final_msg),
             "Quiz Completed!\n\nFinal Score: %d/%d\nAccuracy: %d%%\nFinal "
             "Difficulty: %d\n\nCourse: %s\nTopic: %s",
             end_response.final_results.score,
             end_response.final_results.total_questions,
             end_response.final_results.accuracy,
             end_response.final_results.final_difficulty, current_quiz.course,
             current_quiz.topic);

    GtkWidget *final_dialog =
        gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                               GTK_BUTTONS_OK, "%s", final_msg);
    gtk_dialog_run(GTK_DIALOG(final_dialog));
    gtk_widget_destroy(final_dialog);

    // Free response strings
    if (response.session_id)
      free(response.session_id);
    if (response.feedback)
      free(response.feedback);
    if (end_response.message)
      free(end_response.message);
    if (end_response.final_results.session_id)
      free(end_response.final_results.session_id);

    // Reset all quiz state
    reset_quiz_state();

    // Reset quiz UI
    gtk_widget_hide(quiz_content_box);
    gtk_widget_show_all(quiz_setup_box);
    gtk_entry_set_text(GTK_ENTRY(course_entry), "");
    gtk_entry_set_text(GTK_ENTRY(topic_entry), "");
  } else {
    // Get next question from API
    show_loading_dialog("Loading next question...");

    quiz_start_response_t next_response =
        generate_mcq(&current_quiz, current_student.student_id);

    // Store next question
    mcq_count++;
    if (mcq_count < MAX_MCQS) {
      mcq_t next_mcq = {next_response.question, next_response.options.A,
                        next_response.options.B, next_response.options.C,
                        next_response.options.D};
      mcqs[mcq_count] = next_mcq;
    }

    close_loading_dialog();

    // Free current response strings
    if (response.session_id)
      free(response.session_id);
    if (response.feedback)
      free(response.feedback);

    display_question();
  }
}

void on_end_quiz_clicked(GtkWidget *widget, gpointer data) {
  (void)widget; // Suppress unused parameter warning
  (void)data;   // Suppress unused parameter warning

  char confirm_msg[200];
  snprintf(confirm_msg, sizeof(confirm_msg),
           "Are you sure you want to end the quiz?\n\nCurrent Score: %d/%d",
           current_quiz.score, current_quiz.current_question);

  GtkWidget *dialog =
      gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
                             GTK_BUTTONS_YES_NO, "%s", confirm_msg);

  int response = gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  if (response == GTK_RESPONSE_YES) {
    end_quiz_response_t end_response =
        end_quiz(current_student.student_id, current_quiz.session_id);

    // Final dialog
    char final_msg[400];
    snprintf(final_msg, sizeof(final_msg),
             "Quiz Ended!\n\nFinal Score: %d/%d\nAccuracy: %d%%\nFinal "
             "Difficulty: %d\n\nCourse: %s\nTopic: %s",
             end_response.final_results.score,
             end_response.final_results.total_questions,
             end_response.final_results.accuracy,
             end_response.final_results.final_difficulty, current_quiz.course,
             current_quiz.topic);

    GtkWidget *final_dialog =
        gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                               GTK_BUTTONS_OK, "%s", final_msg);
    gtk_dialog_run(GTK_DIALOG(final_dialog));
    gtk_widget_destroy(final_dialog);

    // Free response strings
    if (end_response.message)
      free(end_response.message);
    if (end_response.final_results.session_id)
      free(end_response.final_results.session_id);

    // Reset all quiz state
    reset_quiz_state();

    // Reset quiz UI
    gtk_widget_hide(quiz_content_box);
    gtk_widget_show_all(quiz_setup_box);
    gtk_entry_set_text(GTK_ENTRY(course_entry), "");
    gtk_entry_set_text(GTK_ENTRY(topic_entry), "");

    // Go back to dashboard
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "dashboard");
  }
}

// Profile analyzer functions

void on_submit_profile_clicked(GtkWidget *widget, gpointer data) {
  (void)widget; // Suppress unused parameter warning
  (void)data;   // Suppress unused parameter warning

  const char *cgpa_str = gtk_entry_get_text(GTK_ENTRY(cgpa_entry));
  const char *major = gtk_entry_get_text(GTK_ENTRY(major_entry));

  GtkTextBuffer *buffer;
  GtkTextIter start, end;

  // Get short-term goals
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(short_term_goals_text));
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  char *short_term_goals =
      gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

  // Get long-term goals
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(long_term_goals_text));
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  char *long_term_goals = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

  // Get industries of interest
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(industries_of_interest_text));
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  char *industries_of_interest =
      gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

  // Get target roles
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(target_roles_text));
  gtk_text_buffer_get_bounds(buffer, &start, &end);
  char *target_roles = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

  // Validate inputs
  if (strlen(cgpa_str) == 0 || strlen(major) == 0 ||
      strlen(short_term_goals) == 0 || strlen(long_term_goals) == 0 ||
      strlen(industries_of_interest) == 0 || strlen(target_roles) == 0) {
    GtkWidget *dialog =
        gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                               GTK_BUTTONS_OK, "Please fill in all fields!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(short_term_goals);
    g_free(long_term_goals);
    g_free(industries_of_interest);
    g_free(target_roles);
    return;
  }

  // Parse CGPA
  current_profile.cgpa = atof(cgpa_str);

  if (current_profile.cgpa < 0.0 || current_profile.cgpa > 4.0) {
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
        "CGPA must be between 0.0 and 4.0!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(short_term_goals);
    g_free(long_term_goals);
    g_free(industries_of_interest);
    g_free(target_roles);
    return;
  }

  // Store values in current_profile
  strncpy(current_profile.major, major, DETAILS_LENGTH - 1);
  current_profile.major[DETAILS_LENGTH - 1] = '\0';
  strncpy(current_profile.short_term_goals, short_term_goals,
          DETAILS_LENGTH - 1);
  current_profile.short_term_goals[DETAILS_LENGTH - 1] = '\0';
  strncpy(current_profile.long_term_goals, long_term_goals, DETAILS_LENGTH - 1);
  current_profile.long_term_goals[DETAILS_LENGTH - 1] = '\0';
  strncpy(current_profile.industries_of_interest, industries_of_interest,
          DETAILS_LENGTH - 1);
  current_profile.industries_of_interest[DETAILS_LENGTH - 1] = '\0';
  strncpy(current_profile.target_roles, target_roles, DETAILS_LENGTH - 1);
  current_profile.target_roles[DETAILS_LENGTH - 1] = '\0';

  // Call the API
  show_loading_dialog("Analyzing your profile...\nPlease wait...");

  pf_analyzer_request_t request = {
      .student_id = current_student.student_id,
      .cgpa = current_profile.cgpa,
      .major = current_profile.major,
      .short_term_goals = current_profile.short_term_goals,
      .long_term_goals = current_profile.long_term_goals,
      .industries_of_interest = current_profile.industries_of_interest,
      .target_roles = current_profile.target_roles};

  pf_analyzer_response_t response = pf_analyzer(&request);

  close_loading_dialog();

  // Free allocated strings
  g_free(short_term_goals);
  g_free(long_term_goals);
  g_free(industries_of_interest);
  g_free(target_roles);

  // Check if we got valid feedback
  if (response.feedback) {
    // Save response to markdown file
    int save_result = save_pf_analyzer_response_to_file(&response,
                                          current_student.student_id);
    
    // Convert markdown feedback to Pango markup
    char* pango_feedback = markdown_to_pango(response.feedback);
    
    // Prepare feedback with save notification
    char *full_feedback = NULL;
    if (save_result == 0) {
      printf(ANSI_FG_GREEN "Profile analysis saved successfully!\n" ANSI_RESET);
      
      if (pango_feedback) {
        // Add bold notification at the top of feedback
        const char* notification = "<b>✓ Your Profile Analysis has been saved at \"astrogon/profile_analysis/\" folder</b>\n\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
        size_t notification_len = strlen(notification);
        size_t feedback_len = strlen(pango_feedback);
        full_feedback = malloc(notification_len + feedback_len + 1);
        
        if (full_feedback) {
          strcpy(full_feedback, notification);
          strcat(full_feedback, pango_feedback);
          gtk_label_set_markup(GTK_LABEL(feedback_label), full_feedback);
          free(full_feedback);
        } else {
          gtk_label_set_markup(GTK_LABEL(feedback_label), pango_feedback);
        }
        free(pango_feedback);
      } else {
        gtk_label_set_text(GTK_LABEL(feedback_label), response.feedback);
      }
    } else {
      fprintf(stderr, ANSI_FG_YELLOW
              "Warning: Failed to save profile analysis to file\n" ANSI_RESET);
      if (pango_feedback) {
        gtk_label_set_markup(GTK_LABEL(feedback_label), pango_feedback);
        free(pango_feedback);
      } else {
        gtk_label_set_text(GTK_LABEL(feedback_label), response.feedback);
      }
    }

    // Show feedback
    gtk_widget_hide(profile_form_box);
    gtk_widget_show_all(profile_feedback_box);

    // Free response
    free_pf_analyzer_response(&response);
  } else {
    // Show error
    char error_msg[500];
    if (response.message) {
      snprintf(error_msg, sizeof(error_msg), "Error: %s", response.message);
      free_pf_analyzer_response(&response);
    } else {
      strcpy(error_msg, "Failed to analyze profile. Please try again.");
    }

    GtkWidget *dialog =
        gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                               GTK_BUTTONS_OK, "%s", error_msg);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
}

void on_new_analysis_clicked(GtkWidget *widget, gpointer data) {
  (void)widget; // Suppress unused parameter warning
  (void)data;   // Suppress unused parameter warning

  // Reset form
  gtk_entry_set_text(GTK_ENTRY(cgpa_entry), "");
  gtk_entry_set_text(GTK_ENTRY(major_entry), "");

  GtkTextBuffer *buffer;
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(short_term_goals_text));
  gtk_text_buffer_set_text(buffer, "", -1);

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(long_term_goals_text));
  gtk_text_buffer_set_text(buffer, "", -1);

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(industries_of_interest_text));
  gtk_text_buffer_set_text(buffer, "", -1);

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(target_roles_text));
  gtk_text_buffer_set_text(buffer, "", -1);

  // Show form
  gtk_widget_hide(profile_feedback_box);
  gtk_widget_show_all(profile_form_box);
}

void on_back_to_dashboard_clicked(GtkWidget *widget, gpointer data) {
  gtk_stack_set_visible_child_name(GTK_STACK(stack), "dashboard");
}

// Build UI
void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *signup_box, *dashboard_box;
  GtkWidget *button;
  GtkWidget *label;

  // Create main window
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Astrogon");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);

  // Set window icon (provide path to your icon file)
  GError *error = NULL;
  if (!gtk_window_set_default_icon_from_file("assets/icon.png", &error)) {
    if (error) {
      g_warning("Failed to load default icon: %s", error->message);
      g_error_free(error);
    }
  }

  // Create stack
  stack = gtk_stack_new();
  gtk_container_add(GTK_CONTAINER(window), stack);

  // === SIGN UP PAGE ===
  signup_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_halign(signup_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(signup_box, GTK_ALIGN_CENTER);

  label = gtk_label_new(NULL);
  gtk_label_set_markup(
      GTK_LABEL(label),
      "<span size='x-large' weight='bold'>Student Sign In/Sign Up</span>");
  gtk_box_pack_start(GTK_BOX(signup_box), label, FALSE, FALSE, 20);

  label = gtk_label_new("Student ID (Format: ##K-####):");
  gtk_box_pack_start(GTK_BOX(signup_box), label, FALSE, FALSE, 0);

  id_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(id_entry), "e.g., 25K-0119");
  gtk_entry_set_max_length(GTK_ENTRY(id_entry), ID_LENGTH - 1);
  gtk_box_pack_start(GTK_BOX(signup_box), id_entry, FALSE, FALSE, 0);

  label = gtk_label_new("Password:");
  gtk_box_pack_start(GTK_BOX(signup_box), label, FALSE, FALSE, 0);

  password_entry = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
  gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Enter password");
  gtk_entry_set_max_length(GTK_ENTRY(password_entry), PASSWORD_LENGTH - 1);
  gtk_box_pack_start(GTK_BOX(signup_box), password_entry, FALSE, FALSE, 0);

  button = gtk_button_new_with_label("Sign In / Sign Up");
  gtk_widget_set_size_request(button, 200, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_signin_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(signup_box), button, FALSE, FALSE, 10);

  gtk_stack_add_named(GTK_STACK(stack), signup_box, "signup");

  // === DASHBOARD PAGE ===
  dashboard_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_halign(dashboard_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(dashboard_box, GTK_ALIGN_CENTER);

  info_label = gtk_label_new("Student Information");
  gtk_label_set_justify(GTK_LABEL(info_label), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(dashboard_box), info_label, FALSE, FALSE, 20);

  GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);

  button = gtk_button_new_with_label("Quiz");
  gtk_widget_set_size_request(button, 120, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_page1_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(button_box), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label("Pf. Analyzer");
  gtk_widget_set_size_request(button, 120, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_page2_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(button_box), button, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(dashboard_box), button_box, FALSE, FALSE, 0);

  button = gtk_button_new_with_label("Logout");
  gtk_widget_set_size_request(button, 120, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_logout_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(dashboard_box), button, FALSE, FALSE, 10);

  gtk_stack_add_named(GTK_STACK(stack), dashboard_box, "dashboard");

  // === PAGE 1 - QUIZ APPLICATION ===
  GtkWidget *page1_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  // Quiz setup box (course and topic entry)
  quiz_setup_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_halign(quiz_setup_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(quiz_setup_box, GTK_ALIGN_CENTER);

  label = gtk_label_new(NULL);
  gtk_label_set_markup(
      GTK_LABEL(label),
      "<span size='x-large' weight='bold'>Quiz Application</span>");
  gtk_box_pack_start(GTK_BOX(quiz_setup_box), label, FALSE, FALSE, 20);

  label = gtk_label_new("Course Name:");
  gtk_box_pack_start(GTK_BOX(quiz_setup_box), label, FALSE, FALSE, 0);

  course_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(course_entry),
                                 "e.g., Computer Science");
  gtk_widget_set_size_request(course_entry, 300, -1);
  gtk_box_pack_start(GTK_BOX(quiz_setup_box), course_entry, FALSE, FALSE, 0);

  label = gtk_label_new("Topic Name:");
  gtk_box_pack_start(GTK_BOX(quiz_setup_box), label, FALSE, FALSE, 0);

  topic_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(topic_entry),
                                 "e.g., Data Structures");
  gtk_widget_set_size_request(topic_entry, 300, -1);
  gtk_box_pack_start(GTK_BOX(quiz_setup_box), topic_entry, FALSE, FALSE, 0);

  button = gtk_button_new_with_label("Start Quiz");
  gtk_widget_set_size_request(button, 200, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_start_quiz_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(quiz_setup_box), button, FALSE, FALSE, 10);

  button = gtk_button_new_with_label("Back to Dashboard");
  gtk_widget_set_size_request(button, 200, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_back_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(quiz_setup_box), button, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(page1_container), quiz_setup_box, TRUE, TRUE, 0);

  // Quiz content box (MCQs)
  quiz_content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_widget_set_halign(quiz_content_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(quiz_content_box, GTK_ALIGN_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(quiz_content_box), 20);

  question_label = gtk_label_new("Question will appear here");
  gtk_label_set_line_wrap(GTK_LABEL(question_label), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(question_label), 60);
  gtk_label_set_justify(GTK_LABEL(question_label), GTK_JUSTIFY_LEFT);
  gtk_widget_set_halign(question_label, GTK_ALIGN_START);
  gtk_label_set_use_markup(GTK_LABEL(question_label), TRUE);
  gtk_box_pack_start(GTK_BOX(quiz_content_box), question_label, FALSE, FALSE,
                     10);

  // Radio buttons for options
  GtkWidget *options_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
  gtk_widget_set_halign(options_box, GTK_ALIGN_START);

  radio_a = gtk_radio_button_new_with_label(NULL, "Option A");
  gtk_box_pack_start(GTK_BOX(options_box), radio_a, FALSE, FALSE, 0);

  radio_b = gtk_radio_button_new_with_label_from_widget(
      GTK_RADIO_BUTTON(radio_a), "Option B");
  gtk_box_pack_start(GTK_BOX(options_box), radio_b, FALSE, FALSE, 0);

  radio_c = gtk_radio_button_new_with_label_from_widget(
      GTK_RADIO_BUTTON(radio_a), "Option C");
  gtk_box_pack_start(GTK_BOX(options_box), radio_c, FALSE, FALSE, 0);

  radio_d = gtk_radio_button_new_with_label_from_widget(
      GTK_RADIO_BUTTON(radio_a), "Option D");
  gtk_box_pack_start(GTK_BOX(options_box), radio_d, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(quiz_content_box), options_box, FALSE, FALSE, 0);

  // Action buttons
  GtkWidget *quiz_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(quiz_button_box, GTK_ALIGN_CENTER);

  button = gtk_button_new_with_label("Submit Answer");
  gtk_widget_set_size_request(button, 150, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_submit_answer_clicked),
                   NULL);
  gtk_box_pack_start(GTK_BOX(quiz_button_box), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label("End Quiz");
  gtk_widget_set_size_request(button, 150, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_end_quiz_clicked), NULL);
  gtk_box_pack_start(GTK_BOX(quiz_button_box), button, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(quiz_content_box), quiz_button_box, FALSE, FALSE,
                     10);

  gtk_box_pack_start(GTK_BOX(page1_container), quiz_content_box, TRUE, TRUE, 0);
  gtk_widget_hide(quiz_content_box);

  gtk_stack_add_named(GTK_STACK(stack), page1_container, "page1");

  // === PAGE 2 - PROFILE ANALYZER ===
  GtkWidget *page2_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  // Profile form box
  profile_form_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_halign(profile_form_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(profile_form_box, GTK_ALIGN_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(profile_form_box), 20);

  label = gtk_label_new(NULL);
  gtk_label_set_markup(
      GTK_LABEL(label),
      "<span size='x-large' weight='bold'>Student Profile Analyzer</span>");
  gtk_box_pack_start(GTK_BOX(profile_form_box), label, FALSE, FALSE, 10);

  label = gtk_label_new("Enter your CGPA (0.0 - 4.0):");
  gtk_box_pack_start(GTK_BOX(profile_form_box), label, FALSE, FALSE, 0);

  cgpa_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(cgpa_entry), "e.g., 3.5");
  gtk_widget_set_size_request(cgpa_entry, 400, -1);
  gtk_box_pack_start(GTK_BOX(profile_form_box), cgpa_entry, FALSE, FALSE, 0);

  label = gtk_label_new("Major/Field of Study:");
  gtk_box_pack_start(GTK_BOX(profile_form_box), label, FALSE, FALSE, 5);

  major_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(major_entry),
                                 "e.g., Computer Science");
  gtk_widget_set_size_request(major_entry, 400, -1);
  gtk_box_pack_start(GTK_BOX(profile_form_box), major_entry, FALSE, FALSE, 0);

  label = gtk_label_new("Short-term Career Goals:");
  gtk_box_pack_start(GTK_BOX(profile_form_box), label, FALSE, FALSE, 5);

  GtkWidget *scrolled1 = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled1),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scrolled1, 400, 60);

  short_term_goals_text = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(short_term_goals_text),
                              GTK_WRAP_WORD);
  gtk_container_add(GTK_CONTAINER(scrolled1), short_term_goals_text);
  gtk_box_pack_start(GTK_BOX(profile_form_box), scrolled1, FALSE, FALSE, 0);

  label = gtk_label_new("Long-term Career Goals:");
  gtk_box_pack_start(GTK_BOX(profile_form_box), label, FALSE, FALSE, 5);

  GtkWidget *scrolled2 = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled2),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scrolled2, 400, 60);

  long_term_goals_text = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(long_term_goals_text),
                              GTK_WRAP_WORD);
  gtk_container_add(GTK_CONTAINER(scrolled2), long_term_goals_text);
  gtk_box_pack_start(GTK_BOX(profile_form_box), scrolled2, FALSE, FALSE, 0);

  label = gtk_label_new("Industries of Interest:");
  gtk_box_pack_start(GTK_BOX(profile_form_box), label, FALSE, FALSE, 5);

  GtkWidget *scrolled3 = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled3),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scrolled3, 400, 60);

  industries_of_interest_text = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(industries_of_interest_text),
                              GTK_WRAP_WORD);
  gtk_container_add(GTK_CONTAINER(scrolled3), industries_of_interest_text);
  gtk_box_pack_start(GTK_BOX(profile_form_box), scrolled3, FALSE, FALSE, 0);

  label = gtk_label_new("Target Job Roles/Positions:");
  gtk_box_pack_start(GTK_BOX(profile_form_box), label, FALSE, FALSE, 5);

  GtkWidget *scrolled4 = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled4),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scrolled4, 400, 60);

  target_roles_text = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(target_roles_text), GTK_WRAP_WORD);
  gtk_container_add(GTK_CONTAINER(scrolled4), target_roles_text);
  gtk_box_pack_start(GTK_BOX(profile_form_box), scrolled4, FALSE, FALSE, 0);

  GtkWidget *profile_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(profile_button_box, GTK_ALIGN_CENTER);

  button = gtk_button_new_with_label("Analyze Profile");
  gtk_widget_set_size_request(button, 150, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_submit_profile_clicked),
                   NULL);
  gtk_box_pack_start(GTK_BOX(profile_button_box), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label("Back to Dashboard");
  gtk_widget_set_size_request(button, 150, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_back_to_dashboard_clicked),
                   NULL);
  gtk_box_pack_start(GTK_BOX(profile_button_box), button, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(profile_form_box), profile_button_box, FALSE,
                     FALSE, 10);

  gtk_box_pack_start(GTK_BOX(page2_container), profile_form_box, TRUE, TRUE, 0);

  // Profile feedback box
  profile_feedback_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_set_border_width(GTK_CONTAINER(profile_feedback_box), 20);

  GtkWidget *feedback_scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(feedback_scrolled),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  feedback_label = gtk_label_new("Feedback will appear here");
  gtk_label_set_line_wrap(GTK_LABEL(feedback_label), TRUE);
  gtk_label_set_selectable(GTK_LABEL(feedback_label), TRUE);
  gtk_label_set_justify(GTK_LABEL(feedback_label), GTK_JUSTIFY_LEFT);
  gtk_label_set_use_markup(GTK_LABEL(feedback_label), TRUE);
  gtk_widget_set_halign(feedback_label, GTK_ALIGN_START);
  gtk_widget_set_valign(feedback_label, GTK_ALIGN_START);
  gtk_widget_set_margin_start(feedback_label, 10);
  gtk_widget_set_margin_end(feedback_label, 10);
  gtk_widget_set_margin_top(feedback_label, 10);
  gtk_widget_set_margin_bottom(feedback_label, 10);

  // gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(feedback_scrolled),
  //                                       feedback_label);
  gtk_container_add(GTK_CONTAINER(feedback_scrolled), feedback_label);
  gtk_box_pack_start(GTK_BOX(profile_feedback_box), feedback_scrolled, TRUE,
                     TRUE, 0);

  GtkWidget *feedback_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(feedback_button_box, GTK_ALIGN_CENTER);

  button = gtk_button_new_with_label("New Analysis");
  gtk_widget_set_size_request(button, 150, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_new_analysis_clicked),
                   NULL);
  gtk_box_pack_start(GTK_BOX(feedback_button_box), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label("Back to Dashboard");
  gtk_widget_set_size_request(button, 150, 40);
  g_signal_connect(button, "clicked", G_CALLBACK(on_back_to_dashboard_clicked),
                   NULL);
  gtk_box_pack_start(GTK_BOX(feedback_button_box), button, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(profile_feedback_box), feedback_button_box, FALSE,
                     FALSE, 10);

  gtk_box_pack_start(GTK_BOX(page2_container), profile_feedback_box, TRUE, TRUE,
                     0);
  gtk_widget_hide(profile_feedback_box);

  gtk_stack_add_named(GTK_STACK(stack), page2_container, "page2");

  // Show all widgets
  gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.example.studentapp",
                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}