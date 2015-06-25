/* vector.c */
#include "vector.h"
#include <stdio.h>
#include <string.h>

size_t vwrite( char *ptr, size_t size, size_t nmemb, void *userdata)
{
  //cast void* to vector type
  struct vector *vec = (struct vector*) userdata;

  //Get newsize
  size_t realsize = size * nmemb,
         newsize  = vec->size + realsize;
  //Reallocate to newsize
  vec->memory = (char *)realloc(vec->memory, newsize + 1);

  //Check if we got memory successfully
  if(vec->memory == NULL)
  {
    fprintf(stderr,"out of memory\n");
    return 0;
  }

  memcpy(vec->memory + vec->size, ptr, realsize);
  vec->size  = newsize;
  vec->memory[ newsize ] = '\0';
  printf("%ld bytes...           \r", vec->size);
  fflush(stdout);
  return nmemb;
}

void   vclear( struct vector *v )
{
  if(v->memory) free(v->memory);
  //set it all to 0
  memset(v,0,sizeof(*v));
}

