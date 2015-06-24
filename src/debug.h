/* printcookie.h */
#pragma once

#ifdef NDEBUG

#define print_cookies(X)

#else

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

#define print_cookies(X) \
{ \
  CURLcode res; \
  struct curl_slist *cookies, *nc; \
  int i = 1; \
  printf("Cookies that this session has:\n"); \
  res = curl_easy_getinfo(X, CURLINFO_COOKIELIST, &cookies); \
  if (res != CURLE_OK) { \
    fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res)); \
    exit(1); \
  } \
  nc = cookies; \
  while (nc) { \
    printf("[%d]: %s\n", i, nc->data); \
    nc = nc->next; \
    i++; \
  } \
  if (i == 1) { \
    printf("(none)\n"); \
  } \
  curl_slist_free_all(cookies); \
} \

#endif
