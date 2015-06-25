/* illust_parse.c */
//TODO add comment parser
//TODO and upload user info parser

#include "illust_parse.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/* PREFIX/SUFFIX Strings to search for */
//Suffix for all important metadata
#define PREFIX_ALL "<div class=\"column-header\">"

#define PREFIX_DATE  "<ul class=\"meta\">"

#define PREFIX_TITLE "</ul><h1 class=\"title\">"
#define SUFFIX_TITLE "</h1>"

#define PREFIX_COMMENT "<p class=\"caption\">"
#define SUFFIX_COMMENT "</p>"

#define PREFIX_TAGUL "<ul class=\"tags\">"
#define SUFFIX_TAGUL "</ul>"

#define PREFIX_TAG   "class=\"text\">"
#define SUFFIX_TAG   "</a>"

#define SUFFIX_IMG   "class=\"original-image\">"

#define PREFIX_MANGA_IMG "<img"

/* Error checking */
#ifndef NDEBUG
#define CHECKPARSE(X) if(!(X))\
{ \
  printf("Failed processing @" __FILE__ ":__LINE__\n"); \
  return PARSE_FAILURE; \
}
#endif

int pixivtool_illust_parse( struct pixivtool_illust   *illust,
                            const  char               *page )
{
  assert(page != NULL);
  assert(illust != NULL);
//page = strstr(page, PREFIX_ALL) + sizeof(PREFIX_ALL) - 1;
  const char
    *start,
    *end;
  int length,
      count;
  struct slist *tag_ptr = NULL;

  /* Find DATE */
  start = strstr(page, PREFIX_DATE);
  CHECKPARSE(start != NULL);
  start += sizeof(PREFIX_DATE) - 1;

  /* Process date */
  count = sscanf(start, " <li> %d年%d月%d日 %d:%d </li> <li> %n",
      &illust->dateTime.year, &illust->dateTime.month,
      &illust->dateTime.day , &illust->dateTime.hour,
      &illust->dateTime.min , &length);
  start += length;
  CHECKPARSE(count == 5);

  /* Get size of Image, OR number of images,
   * which is determined by the count result */
  count = sscanf(start, " %d × %d", &illust->size.width, &illust->size.height);
  /* If this is a single image illustration */
  if(count == 2)
  {
    //Then find url
    illust->number = 1;
    //Search for end of image
    start = strstr(page, SUFFIX_IMG);
    //Search backwards to find the "..." binding the url
    while(*--start != '"');
    end = start;
    while(*--start != '"');
    ++start;
    //Create and set url string
    length = end - start;
    illust->image = (char *)malloc(length+1);
    //copy it over
    strncpy(illust->image, start, length);
    illust->image[length] = '\0';
  }
  /* If this is a multi image illustration  find number of pages*/
  if(count < 2)
  {
    count = sscanf(start, " %*[^0-9] %dP", &illust->number);
    CHECKPARSE(count > 0);
  }

  /* Search for title, find parts of it */
  start = strstr(page,  PREFIX_TITLE);
  CHECKPARSE(start != NULL);
  start +=  + sizeof(PREFIX_TITLE) - 1;
  end   = strstr(start, SUFFIX_TITLE);
  CHECKPARSE(end   != NULL);
  //Get length of string, allocate room for it in illustration title, and copy
  //it
  length = end - start;
  illust->title = (char *)malloc(length + 1);
  strncpy(illust->title, start, length);
  illust->title[length] = '\0';

  /* Search for comment : start searching using title start , because
   * of false positive earlire in webpage*/
  start = strstr(start,  PREFIX_COMMENT);
  /* Skip if there is no comment */
  if(start != NULL)
  {
    //move start to end of prefix
    start += sizeof(PREFIX_COMMENT) - 1;
    end   = strstr(start, SUFFIX_COMMENT);
    //Get string length, allocate and copy
    length = end - start;
    illust->comment = (char *)malloc(length + 1);
    strncpy(illust->comment, start, length);
    illust->comment[length] = '\0';
  }

  /* Search for tags, by finding the bounding area */
  start = strstr(page,  PREFIX_TAGUL);
  //Error check it
  CHECKPARSE(start != NULL);
  //move start to end
  start += sizeof(PREFIX_TAGUL) - 1;
  //Search for end
  end   = strstr(start, SUFFIX_TAGUL);

  //error check it
  CHECKPARSE(end != NULL);

  //Move to first prefix tag
  start = strstr(start, PREFIX_TAG);
  while(start < end && start != NULL)
  {
    //Move start to end of prefix_tag
    start += sizeof(PREFIX_TAG) - 1;

    //Find tag length between PREFIX_TAG....SUFFIX_TAG
    const char *tag_end = strstr(start, SUFFIX_TAG);
    if(tag_end == NULL) break;
    length = tag_end - start;

    //Create and set new linked list
    if(tag_ptr == NULL) tag_ptr = &illust->tags;
    else tag_ptr = tag_ptr->next = (struct slist *)malloc(sizeof(*tag_ptr));
    tag_ptr->next = NULL;

    //Create string
    tag_ptr->string = (char *)malloc(length + 1);

    //Copy tag
    strncpy(tag_ptr->string, start, length);
    tag_ptr->string[length] = '\0';

    //Find next tag
    start = strstr(start, PREFIX_TAG);
  }

  if(illust->number > 1)
    return PARSE_MANGA;
  return PARSE_SUCCESS;
}

int pixivtool_illust_manga( struct pixivtool_illust *illust,
                            const  char             *page )
{
  const char
    *start = strstr(page, PREFIX_MANGA_IMG),
    *end;
  //Find bounds of url "..."
  //start++ makes sure it points to the character after the "
  while(*start++ != '"');
  end = start;
  while(*++end != '"');
  //Get length, create string of said length, copy url into it
  int length = end - start;
  illust->image = (char *)malloc(length + 1);
  strncpy(illust->image, start, length);
  illust->image[length] = '\0';
  return PARSE_SUCCESS;
}

