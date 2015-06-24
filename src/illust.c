/* member_illust.c */
#include "illust.h"
#include "curlcommon.h"
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

struct strarray
{
  char *memory;
  size_t size;
};

//CURLOPT_WRITEFUNCTION function used to fill illust with metadata
//pixivtool_member_illust* is passed into userdata
static size_t strwrite( char *ptr, size_t size, size_t nmemb, void *userdata);


int pixivtool_illust_init( struct pixivtool_account *account,
                           struct pixivtool_illust  *illust,
                           unsigned int id)
{
  assert(account != NULL && illust != NULL);
  CURL *curl = account->session;

  /* Holds downloaded data */
  struct strarray sarray = { NULL, 0 };

  /* Clear and set Illustration data */
  memset(illust, 0, sizeof(*illust));
  illust->id      = id;

  /* Return code */
  int retcode;

  /* Set URL for request */
  char url[512];
  sprintf(url, "%s%d", MEMBER_ILLUST_URL, id);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  /* Set function to handle downloaded data */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, strwrite);

  /* create string to be filled with loaded data */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sarray);

  /* Send request and get data */
  retcode = common_curl_perform(curl);

  /* Reset request */
  curl_easy_reset(curl);

  //Process returned data
  char  *string = sarray.memory, *end;
  int    read, found;
  struct slist *tag_ptr = NULL;

#define PREFIX_DATE  "<ul class=\"meta\">"
#define PREFIX_TITLE "<h1 class=\"title\">"
#define SUFFIX_TITLE "</h1>"
#define PREFIX_TAGUL  "<ul class=\"tags\">"
#define SUFFIX_TAGUL  "</ul>"
#define PREFIX_TAG   "class=\"text\">"
#define SUFFIX_TAG   "</a>"
#define SUFFIX_IMG   "class=\"original-image\">"

#define CHECKFAIL(X) \
  if(X) { \
    printf("Failed processing @" __FILE__ ":__LINE__\n"); \
    retcode = COMMON_FAILURE; \
    goto end; \
  }

  /* Search for prefix string before %d年%d月%d日 */
  string = strstr(string, PREFIX_DATE) + sizeof(PREFIX_DATE) - 1;
  CHECKFAIL(string == NULL);

  found  = sscanf(string, " <li> %d年%d月%d日 %d:%d </li> <li> %n",
      &illust->dateTime.year, &illust->dateTime.month,
      &illust->dateTime.day , &illust->dateTime.hour,
      &illust->dateTime.min , &read);
  string += read;
  //If we dont match properly stop processing
  CHECKFAIL(found < 5);

  /* Get Number of images/ size of image */
  if(sscanf(string," 複数枚投稿 %dP %n", &illust->number, &read) < 1)
  {
    //If single image mode
    illust->number = 1;
    sscanf(string," %d × %d %n",
        &illust->size.width, &illust->size.height, &read);
  }

  string += read;

  /* Search for title */
  string = strstr(string, PREFIX_TITLE) + sizeof(PREFIX_TITLE) - 1;
  CHECKFAIL(string == NULL);
  {
    size_t title_len = strstr(string, SUFFIX_TITLE) - string;
    illust->title = (char *)malloc(title_len + 1);
    strncpy(illust->title, string, title_len);
    illust->title[title_len] = '\0';
  }

  /* Search for tags */
  /* Find bounds of <ul class="tags"> .. </ul> */
  string    = strstr(string, PREFIX_TAGUL) + sizeof(PREFIX_TAGUL) - 1;
  CHECKFAIL(string == NULL);
  end       = strstr(string, SUFFIX_TAGUL);
  CHECKFAIL(end   == NULL);
  /* Go through each tag */
  string    = strstr(string, PREFIX_TAG) + sizeof(PREFIX_TAG) - 1;
  while(string < end && string)
  {
    //Find tag length
    int tag_len = strstr(string, SUFFIX_TAG) - string;

    //Set tag pointer to top of illustration tags
    if(tag_ptr == NULL) tag_ptr = &illust->tags;
    //Or create new tag
    else tag_ptr = tag_ptr->next = (struct slist *)malloc(sizeof(*tag_ptr->next));

    tag_ptr->next = NULL;
    //Create string
    tag_ptr->string = (char *)malloc(tag_len + 1);

    //Copy tag
    strncpy(tag_ptr->string, string, tag_len);
    tag_ptr->string[tag_len] = '\0';

    //Find next tag
    string = strstr(string, PREFIX_TAG) + sizeof(PREFIX_TAG) - 1;
  }
  CHECKFAIL(string == NULL);

  /* Search for the image/manga */
  if(illust->number == 1)
  {
    //Find last part of image string
    string = strstr(string, SUFFIX_IMG);
    //Search backwards and look for the "" surrounding the image url
    while(*--string != '"');
    end = string;
    while(*--string != '"');
    string++;
    size_t url_len = end - string;
    illust->image = (char *)malloc(url_len+1);
    sscanf (string,"%[^\"]", illust->image);
  }
  /* Load from manga_big page */
  else
  {
    //using `page=%d' because pixiv sets the url to whatever %d is. and its
    //easier to use as %d
    sprintf(url, "%s%d&page=%%d", MEMBER_MANGA_URL, id);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* Set function to handle downloaded data */
    free(sarray.memory);
    sarray.memory = NULL;
    sarray.size   =  0;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, strwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA    , &sarray);

    /* Send request and get data */
    retcode = common_curl_perform(curl);

    /* Reset request */
    curl_easy_reset(curl);
#define PREFIX_MANGA_IMG "<img"
    /* Find page */
    char *string = strstr(sarray.memory, PREFIX_MANGA_IMG), *end;
    while(*string++ != '"');
    end = string;
    while(*++end != '"');
    size_t url_len = end - string;
    illust->image = (char *)malloc(url_len+1);
    sscanf(string,"%[^\"]",illust->image);
  }


  //XXX test out what i extraced
  printf("\n======= DEBUG ========\n");
  printf("date: %d年%d月%d日 %d:%d\n",
      illust->dateTime.year, illust->dateTime.month,
      illust->dateTime.day , illust->dateTime.hour,
      illust->dateTime.min);
  printf("size: %dx%d\n",illust->size.width, illust->size.height);
  printf("Images:%d\n",illust->number);
  printf("Title:%s\n", illust->title);
  printf("Tags: ");
  tag_ptr = &illust->tags; found = 0;
  while(tag_ptr) {
    printf(" %d:%s |",found++, tag_ptr->string);
    tag_ptr = tag_ptr->next;
  }
  printf("\nImage: %s\n",illust->image);
  printf("\n======= DEBUG ========\n");

end:

  free(sarray.memory);

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
}



static size_t strwrite(char *ptr, size_t size, size_t nmemb, void *userdata )
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
  printf("%ldbytes...\r",sarr->size);fflush(stdout);
  return nmemb;
}



