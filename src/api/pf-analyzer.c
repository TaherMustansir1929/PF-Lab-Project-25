#include "pf-analyzer.h"
#include "ansi-colors.h"
#include "main.h"
#include "requests.h"
#include <cjson/cJSON.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

pf_analyzer_response_t parse_pf_analyzer_response(const char *json_str) {
  pf_analyzer_response_t response = {0};
  cJSON *root = cJSON_Parse(json_str);
  if (!root) {
    fprintf(stderr, "Failed to parse pf_analyzer_response\n");
    response.message = strdup("Failed to parse JSON response");
    return response;
  }

  // Check for error response with "detail" field
  cJSON *detail = cJSON_GetObjectItemCaseSensitive(root, "detail");
  if (cJSON_IsString(detail)) {
    fprintf(stderr, ANSI_FG_RED "\nServer Error: %s\n" ANSI_RESET,
            detail->valuestring);
    response.message = strdup(detail->valuestring);
    cJSON_Delete(root);
    return response;
  }

  // Parse feedback
  cJSON *feedback = cJSON_GetObjectItemCaseSensitive(root, "feedback");
  if (cJSON_IsString(feedback)) {
    response.feedback = strdup(feedback->valuestring);
  }

  // Parse timestamp
  cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(root, "timestamp");
  if (cJSON_IsString(timestamp)) {
    response.timestamp = strdup(timestamp->valuestring);
  }

  // Parse optional message field
  cJSON *message = cJSON_GetObjectItemCaseSensitive(root, "message");
  if (cJSON_IsString(message)) {
    response.message = strdup(message->valuestring);
  }

  cJSON_Delete(root);
  return response;
}

pf_analyzer_response_t pf_analyzer(const pf_analyzer_request_t *request) {
  // Create empty response object
  pf_analyzer_response_t response = {0};

  // Validate input
  if (!request || !request->student_id || !request->major ||
      !request->short_term_goals || !request->long_term_goals ||
      !request->industries_of_interest || !request->target_roles) {
    fprintf(stderr,
            ANSI_FG_RED "Error: Invalid request parameters\n" ANSI_RESET);
    response.message = strdup("Invalid request parameters");
    return response;
  }

  // Validate CGPA range
  if (request->cgpa < 0.0 || request->cgpa > 10.0) {
    fprintf(stderr, ANSI_FG_RED
            "Error: CGPA must be between 0.0 and 10.0\n" ANSI_RESET);
    response.message = strdup("CGPA must be between 0.0 and 10.0");
    return response;
  }

  //========================
  //===Create JSON object===
  //========================
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "student_id", request->student_id);
  cJSON_AddNumberToObject(json, "cgpa", request->cgpa);
  cJSON_AddStringToObject(json, "major", request->major);
  cJSON_AddStringToObject(json, "short_term_goals", request->short_term_goals);
  cJSON_AddStringToObject(json, "long_term_goals", request->long_term_goals);
  cJSON_AddStringToObject(json, "industries_of_interest",
                          request->industries_of_interest);
  cJSON_AddStringToObject(json, "target_roles", request->target_roles);

  char *json_data = cJSON_PrintUnformatted(json);
  if (!json_data) {
    fprintf(stderr, "Error creating json string\n");
    cJSON_Delete(json);
    response.message = strdup("Error creating json string");
    return response;
  }

  // DEBUG: Print the JSON being sent
  printf("\n\nJSON BEING SENT: %s\n\n", json_data);

  char url[256];
  const char *env = getenv("C_ENV");
  snprintf(url, sizeof(url), "%s%s",
           strcmp(env, "dev") == 0 ? BASE_URL_DEV : BASE_URL_PROD,
           PF_ANALYZER_URL);
  memory_t chunk = post_request(url, json_data);

  if (chunk.err != NULL) {
    fprintf(stderr, "Failed to receive Response: %s\n", chunk.err);
    response.message = strdup(chunk.err);
  } else {
    // DEBUG PRINT
    printf("\n\nRAW CHUNK->RESPONSE: %s\n\n", chunk.response);

    //====================
    //===Parse Response===
    //====================
    response = parse_pf_analyzer_response(chunk.response);

    // Check if we got a valid response (feedback is required for success)
    if (response.feedback == NULL && response.message == NULL) {
      fprintf(stderr, ANSI_FG_RED "\nFailed to get PF analysis.\n" ANSI_RESET);
      response.message = strdup("Failed to get PF analysis");
      free(chunk.response);
      free(json_data);
      cJSON_Delete(json);
      curl_global_cleanup();
      return response;
    }

    // Display success response
    if (response.feedback) {
      printf("\n--------PF Analyzer Response---------\n");
      printf(ANSI_BG_GREEN "Feedback:\n%s\n" ANSI_RESET, response.feedback);
      if (response.timestamp) {
        printf("Timestamp: %s\n", response.timestamp);
      }
      if (response.message) {
        printf("Message: %s\n", response.message);
      }
      printf("------------------------------------\n\n");
    }
  }

  //=============
  //===Cleanup===
  //=============
  free(chunk.response);
  free(json_data);
  cJSON_Delete(json);
  curl_global_cleanup();

  return response;
}

int save_pf_analyzer_response_to_file(const pf_analyzer_response_t *response,
                                      const char *student_id) {
  if (!response || !student_id || !response->feedback || !response->timestamp) {
    fprintf(stderr, ANSI_FG_RED
            "Error: Invalid parameters for saving response\n" ANSI_RESET);
    return -1;
  }

  // Create directory if it doesn't exist
  const char *dir_name = "profile_analysis";
  struct stat st = {0};

  if (stat(dir_name, &st) == -1) {
    if (mkdir(dir_name, 0755) == -1) {
      fprintf(stderr,
              ANSI_FG_RED "Error creating directory '%s': %s\n" ANSI_RESET,
              dir_name, strerror(errno));
      return -1;
    }
    printf(ANSI_FG_GREEN "Created directory: %s\n" ANSI_RESET, dir_name);
  }

  // Create filename: student_id + timestamp
  // Replace colons and spaces in timestamp with underscores for valid filename
  char safe_timestamp[100];
  strncpy(safe_timestamp, response->timestamp, sizeof(safe_timestamp) - 1);
  safe_timestamp[sizeof(safe_timestamp) - 1] = '\0';

  // Replace invalid filename characters
  for (char *p = safe_timestamp; *p; p++) {
    if (*p == ':' || *p == ' ' || *p == '/' || *p == '\\') {
      *p = '_';
    }
  }

  char filename[512];
  snprintf(filename, sizeof(filename), "%s/%s_%s.md", dir_name, student_id,
           safe_timestamp);

  // Open file for writing
  FILE *file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, ANSI_FG_RED "Error opening file '%s': %s\n" ANSI_RESET,
            filename, strerror(errno));
    return -1;
  }

  // Write markdown content
  fprintf(file, "# Student Profile Analysis Report\n\n");
  fprintf(file, "**Student ID:** %s  \n", student_id);
  fprintf(file, "**Analysis Date:** %s  \n\n", response->timestamp);
  fprintf(file, "%s\n\n", response->feedback);
  fprintf(file, "---\n\n");
  fprintf(file, "*This analysis was generated by the PF Analyzer API*\n");

  fclose(file);

  printf(ANSI_FG_GREEN "✓ Profile analysis saved to: %s\n" ANSI_RESET,
         filename);
  return 0;
}

void free_pf_analyzer_response(pf_analyzer_response_t *response) {
  if (!response) {
    return;
  }

  if (response->feedback) {
    free(response->feedback);
    response->feedback = NULL;
  }

  if (response->timestamp) {
    free(response->timestamp);
    response->timestamp = NULL;
  }

  if (response->message) {
    free(response->message);
    response->message = NULL;
  }
}
