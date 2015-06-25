/* member_illust.c */
#include "illust.h"
#include "illust_parse.h"
#include "curlcommon.h"
#include "vector.h"
#include "url.h"

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#define MEMBER_ILLUST_URL \
  "http://www.pixiv.net/member_illust.php?mode=medium&illust_id="

#define MEMBER_MANGA_URL \
  "http://www.pixiv.net/member_illust.php?mode=manga_big&illust_id="


int pixivtool_illust_init( struct pixivtool_account *account,
                           struct pixivtool_illust  *illust,
                           unsigned int id)
{
  assert(account != NULL && illust != NULL);
  CURL *curl = account->session;

  /* Holds downloaded data */
  struct vector vstring = { NULL, 0 };

  /* Clear and set Illustration data */
  memset(illust, 0, sizeof(*illust));
  illust->id = id;

  /* Return code */
  int retcode;

  /* Set URL for request */
  char url[512];
  sprintf(url, "%s%d", MEMBER_ILLUST_URL, id);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  /* Set function to handle downloaded data */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, vwrite);

  /* create string to be filled with loaded data */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,    &vstring);

  /* Send request and get data */
  retcode = common_curl_perform(curl);

  /* Check response code */
  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  if(response_code != 200)
    retcode = COMMON_FAILURE;

  /* Reset request */
  curl_easy_reset(curl);

  if(retcode == COMMON_SUCCESS)
  {
    //Process Data
    int parse = pixivtool_illust_parse( illust, vstring.memory );
    //In case of manga page we need to load it
    if(parse == PARSE_MANGA)
    {
      //Clear vector
      vclear(&vstring);
      //using `page=%d' because pixiv sets the url to whatever %d is. and its
      //easier to use as %d
      sprintf(url, "%s%d&page=%%d", MEMBER_MANGA_URL, id);
      curl_easy_setopt(curl, CURLOPT_URL, url);
      /* Set writefunctions */
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, vwrite);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA    ,&vstring);
      /* Send request and get data */
      retcode = common_curl_perform(curl);
      /* Get response code */
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      if(response_code != 200)
        retcode = COMMON_FAILURE;
      /* Reset request */
      curl_easy_reset(curl);
      pixivtool_illust_manga( illust, vstring.memory );
    }
    else if(parse == PARSE_FAILURE)
      retcode = COMMON_FAILURE;
    vclear(&vstring);
  }

  //TODO test out what i extraced
  //TODO put this somewhere else
  printf("\n======= DEBUG ========\n");
  printf("date: %d年%d月%d日 %d:%d\n",
      illust->dateTime.year, illust->dateTime.month,
      illust->dateTime.day , illust->dateTime.hour,
      illust->dateTime.min);
  printf("size: %dx%d\n",illust->size.width, illust->size.height);
  printf("Images:%d\n",illust->number);
  printf("Title:%s\n", illust->title);
  printf("Comment:%s\n", illust->comment);
  printf("Tags: ");
  struct slist *tag_ptr = &illust->tags;
  int i = 0;
  while(tag_ptr) {
    printf(" %d:%s |",i++, tag_ptr->string);
    tag_ptr = tag_ptr->next;
  }
  printf("\nImage: %s",illust->image);
  printf("\n======= DEBUG ========\n");

  /* Print if failure */
  if(retcode == COMMON_FAILURE)
    printf("Failed to load metadata for member illustration %d\n",id);

  return retcode;
}


int pixivtool_illust_dl( struct pixivtool_account *account,
                         struct pixivtool_illust  *illust )
{
  assert(account != NULL && illust != NULL);
  assert(account->session != NULL);
  assert(illust->image != NULL);
  int   retcode;
  FILE *file = NULL;
  CURL *curl = account->session;

  /* Set function to handle downloaded data */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);

  /* Add cookie for downloading image */
  curl_easy_setopt(curl ,CURLOPT_COOKIE, "pixiv_embed=pix");

  //Downlaod single image file
  if(illust->number == 1)
  {
    /* Open file for writing */
    file = fopen(filename(illust->image),"w+");
    if(file == NULL) {
      printf("Failed to open file\n");
      printf("%s",filename(illust->image));
      return COMMON_FAILURE;
    }

    /* Set Curl request to write to file */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    /* Set url */
    curl_easy_setopt(curl, CURLOPT_URL, illust->image);

    /* Send request and get data */
    retcode = common_curl_perform(curl);

    /* Check resposne code */
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    if(response_code != 200)
      retcode = COMMON_FAILURE;
    else
    {
      double speed, size;
      curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &size);
      curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed);
      printf("Downloaded image (%f bytes | %f bytes/second)\n", size, speed);
    }


  }
  else
  {
    size_t page = 0;
    while(page < illust->number)
    {
      /* Get url from format string url */
      char url[512];
      sprintf(url,illust->image,page);
      curl_easy_setopt(curl, CURLOPT_URL, url);

      /* Set file to use */
      if(file) fclose(file);
      file = fopen(filename(url),"w+");

      /* Set Curl request to write to file */
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

      /* Send request and get data */
      retcode = common_curl_perform(curl);

      /* Check resposne code */
      long response_code;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      if(response_code != 200)
      {
        retcode = COMMON_FAILURE;
        break;
      }
      else
      {
        double speed, size;
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &size);
        curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed);
        printf("Downloaded image %ld (%f bytes | %f bytes/second)\n", page, size, speed);
      }

      /* Get next page */
      page++;
    }
  }

  if(retcode == COMMON_SUCCESS)
    printf("Successfully");
  else
    printf("Failed to");
  printf(" download illustration (%d)\n",illust->id);

  if(file) fclose(file);

  curl_easy_reset(curl);

  return retcode;
}


int pixivtool_illust_cleanup( struct pixivtool_illust  *illust )
{
  assert(illust != NULL);
  //Free strings
  if(illust->title)       free(illust->title);
  if(illust->comment)     free(illust->comment);
  if(illust->image)       free(illust->image);
  if(illust->tags.string) free(illust->tags.string);

  struct slist *top  =  illust->tags.next,
               *prev;
  //Cleanup tags
  while(top)
  {
    if(top->string) free(top->string);
    prev = top;
    top  = top->next;
    free(prev);
  }

  //Reset data
  memset(illust, 0, sizeof(*illust));
  return 0;
}


