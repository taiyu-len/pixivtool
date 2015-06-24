/*!@file init.h
 * Initilization functions.
 * Functions to be called once at the beginning of the program,
 * as they are only called once, all functions are inline
 * */
#pragma once
#include "login.h"

#include <curl/curl.h>
#include <stdlib.h>
#include <assert.h>

inline void pixivtool_init(void)
{
  //Make sure this only gets called once
#ifndef NDEBUG
  static int once = 1;
  assert(once);
  once = 0;
#endif
  /* Initilize curl */
  curl_global_init(CURL_GLOBAL_ALL);

  /* Set atexit functions */
  atexit(curl_global_cleanup);
  atexit(pixivtool_logout);
}
