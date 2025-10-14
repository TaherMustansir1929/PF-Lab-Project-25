#include "requests.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

//================================
//===Callback for response data===
//================================
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t total_size = size * nmemb;
  memory_t *mem = (memory_t *)userp;

  char *ptr = realloc(mem->response, mem->size + total_size + 1);
  if (ptr == NULL)
  {
    fprintf(stderr, "Not enough memory to store response\n");
    return 0;
  }

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), contents, total_size);
  mem->size += total_size;
  mem->response[mem->size] = '\0';

  return total_size;
}

//=====================
//===Handle requests===
//=====================
memory_t post_request(const char *url, const char *json_data)
{
  memory_t chunk;
  chunk.response = malloc(0);
  chunk.size = 0;
  chunk.err = NULL;
  //=====================
  //===Initialize CURL===
  //=====================
  CURL *curl = curl_easy_init();
  if (curl)
  {
    CURLcode res;
    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Debugging
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
      chunk.err = malloc(255);
      snprintf(chunk.err, 255, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
      return chunk;
    }
    else
    {
      return chunk;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
  else
  {
    fprintf(stderr, "Failed to initialize CURL\n");
    chunk.err = malloc(255);
    snprintf(chunk.err, 255, "Failed to initialize CURL\n");
    return chunk;
  }
}


memory_t get_request(const char *url){
  memory_t chunk;
  chunk.response = malloc(0);
  chunk.size = 0;
  chunk.err = NULL;

  //=====================
  //===Initialize CURL===
  //=====================
  CURL *curl = curl_easy_init();
  if (curl)
  {
    CURLcode res;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Debugging
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
      chunk.err = malloc(255);
      snprintf(chunk.err, 255, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
      return chunk;
    }
    else
    {
      return chunk;
    }
    curl_easy_cleanup(curl);
  }
  else
  {
    fprintf(stderr, "Failed to initialize CURL\n");
    chunk.err = malloc(255);
    snprintf(chunk.err, 255, "Failed to initialize CURL\n");
    return chunk;
  }
}
