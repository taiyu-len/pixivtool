/* curlcommon.c */
#include "curlcommon.h"

#include <string.h>

static char errbuf[CURL_ERROR_SIZE];

int common_curl_perform( CURL *curl )
{
  /* Set error buffer */
  errbuf[0] = 0;
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
  /* Perform request */
  CURLcode res = curl_easy_perform(curl);
  /* Check for failure */
  if(res != CURLE_OK)
  {
    size_t len = strlen(errbuf);
    fprintf(stderr, "\nFailed to login: libcurl(%d) ",res);
    if(len)
      fprintf(stderr, "%s%s", errbuf, ((errbuf[len-1] != '\n') ? "\n" : ""));
    else
      fprintf(stderr, "%s\n", curl_easy_strerror(res));
    return COMMON_FAILURE;
  }
  return COMMON_SUCCESS;
}

