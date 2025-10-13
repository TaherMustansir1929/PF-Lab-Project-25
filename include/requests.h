#ifndef REQUESTS_H
#define REQUESTS_H

#include <stdio.h>
#include <curl/curl.h>

typedef struct Memory
{
  char *response;
  size_t size;
  char *err;
} memory_t;

memory_t post_request(const char *url, const char *json_data);
memory_t get_request(const char *url);

#endif
