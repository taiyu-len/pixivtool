/* url.h */
#pragma once
#include <string.h>

char *filename(char *url)
{
  return rindex(url,'/') + 1;
}

