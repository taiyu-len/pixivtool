/* member_illust.c */
#include "member_illust.h"
#include "curlcommon.h"

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#define MEMBER_ILLUST_URL \
  "http://www.pixiv.net/member_illust.php?mode=medium&illust_id="

struct strarray
{
  char *memory;
  size_t size;
};

//CURLOPT_WRITEFUNCTION function used to fill illust with metadata
//pixivtool_member_illust* is passed into userdata
static size_t loadmeta( char *ptr, size_t size, size_t nmemb, void *userdata);


int pixivtool_member_illust_init( struct pixivtool_account *account,
                                  struct pixivtool_member_illust *illust,
                                  unsigned int id)
{
  assert(account != NULL && illust != NULL);
  CURL *curl = account->session;

  /* Set Illust id */
  illust->id = id;

  /* Return code */
  int retcode;

  /* Set URL */
  char url[512];
  sprintf(url, "%s%d", MEMBER_ILLUST_URL, id);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  /* Set function to handle downloaded data */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, loadmeta);

  /* Set string to be filled with loaded data */
  struct strarray sarray = { NULL, 0 };
  curl_easy_setopt(curl, CURLOPT_WRITEDATA    , &sarray);

  /* Send request and get data */
  retcode = common_curl_perform(curl);

  //Process returned data

  /* Search for string before %d年%d月%d日 */
#define PREFIX_DATE "<ul class=\"meta\"><li>"
  char *str = strstr(sarray.memory,PREFIX_DATE) + sizeof(PREFIX_DATE) - 1,
       *end;
  int jmp;
  if(str==NULL) goto end;
  /* Scan string and get data from it */
  if(sscanf(str, "%d年%d月%d日 %d:%d%n",
        &illust->dateTime.year,   &illust->dateTime.month,
        &illust->dateTime.day,    &illust->dateTime.hour,
        &illust->dateTime.minute, &jmp)  <  5)
  {
    retcode = COMMON_FAILURE;
    goto end;
  }
  str += jmp;
  printf("extraced date: %d,%d,%d %d:%d\n",
      illust->dateTime.year,
      illust->dateTime.month,
      illust->dateTime.day,
      illust->dateTime.hour,
      illust->dateTime.minute);
  

end:
  /* Reset request */
  curl_easy_reset(curl);

  /* Print if failure */
  if(retcode == COMMON_FAILURE)
    printf("Failed to load metadata for member illustration %d\n",id);

  return retcode;
}

char *WRITE_BUFFER[1440];

//TODO make it load metadata properly
static size_t loadmeta(char *ptr, size_t size, size_t nmemb, void *userdata )
{
  size_t realsize = size * nmemb;

  struct strarray *sarr = (struct strarray *)userdata;
  sarr->memory = (char *)realloc(sarr->memory, sarr->size + realsize + 1);
  if(sarr->memory == NULL)
  {
    fprintf(stderr,"out of memory\n");
    return 0;
  }
  memcpy(sarr->memory + sarr->size, ptr, realsize);
  sarr->size  += realsize;
  sarr->memory[ sarr->size ] = 0;
  return nmemb;
}



