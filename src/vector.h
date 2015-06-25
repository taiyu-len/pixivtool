/*!@file vector.h
 * Continer to be used in place of fwrite in curl.
 * Usage:
 * curl_easy_setopt(curl, CURLOPT_WRITEDATA,     vector *v);
 * curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, vwrite   );
 * */
#pragma once
#include <stdlib.h>

struct vector
{
  char   *memory;
  size_t size;
};

/*!To be used as the CURLOPT_WRITEFUNCTION  */
size_t vwrite( char *ptr, size_t size, size_t nmemb, void *userdata);
/*! To clear a vector */
void   vclear( struct vector *v);

