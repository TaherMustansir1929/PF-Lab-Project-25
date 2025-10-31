#include "dotenv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_env_file(void) {
  FILE *file = fopen(ENV_FILE, "r");
  if (!file) {
    perror("Could not open .env file");
    return;
  }

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    // Remove newline character
    line[strcspn(line, "\n")] = '\0';

    // Ignore comments and blank lines
    if (line[0] == '#' || strlen(line) == 0)
      continue;

    // Split into key and value
    char *equal_sign = strchr(line, '=');
    if (!equal_sign)
      continue;

    *equal_sign = '\0';
    char *key = line;
    char *value = equal_sign + 1;

    // Set environment variable
    setenv(key, value, 1); // 1 = overwrite if exists
  }

  fclose(file);
}
