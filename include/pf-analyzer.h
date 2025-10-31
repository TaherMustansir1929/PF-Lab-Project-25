#ifndef PF_ANALYZER_H
#define PF_ANALYZER_H

#include <cjson/cJSON.h>

#define PF_ANALYZER_URL "/api/pfanalyzer"

typedef struct PFAnalyzerRequest {
  char *student_id;
  float cgpa;
  char *major;
  char *short_term_goals;
  char *long_term_goals;
  char *industries_of_interest;
  char *target_roles;
} pf_analyzer_request_t;

typedef struct PFAnalyzerResponse {
  char *feedback;
  char *timestamp;
  char *message; // For error messages
} pf_analyzer_response_t;

pf_analyzer_response_t pf_analyzer(const pf_analyzer_request_t *request);
pf_analyzer_response_t parse_pf_analyzer_response(const char *json_str);
int save_pf_analyzer_response_to_file(const pf_analyzer_response_t *response,
                                      const char *student_id);
void free_pf_analyzer_response(pf_analyzer_response_t *response);

#endif // PF_ANALYZER_H