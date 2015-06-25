/*!@file curlcommon.h
 * @brief A set of curl functions.
 * Many reused functions for various curl related things */
#pragma once
#include <curl/curl.h>

enum {
  COMMON_SUCCESS,
  COMMON_FAILURE
};

/*!Safely perform whatever request curl has.
 * @param curl The request handle to be performed
 * @return SUCCESS or FAILURE */

int common_curl_perform( CURL *curl );

