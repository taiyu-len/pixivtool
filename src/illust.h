/*!@file illust.h
 * illustration metadata. Store metadata for illustration, and download link
 * for getting it.
 * TODO fancier save options, formatted file names and whatnot.
 * TODO right now it just saves as 12345_p0.png or whatever*/
#pragma once
#include "account.h"


/*TODO change slist to this. better to keep track of a single object
  TODO (slist|string) instead of 2 objects (slist)->(string)
struct slist {
  struct slist *next;
  char         string[];
};
*/

struct slist {
  struct slist *next;
  char         *string;
};

//!Stores meta data for member illustration
struct pixivtool_illust
{
  //!ID of the member illust.
  unsigned int id;

  //!Number of images, n>1 is multiimage.
  unsigned int number;

  //!Title of member illust.
  char *title;

  //!Comment for the member illust.
  char *comment;

  //!Date this member illust is uploaded.
  struct {
    int year,  //!< Starts at 1970
        month,
        day,
        hour,
        min;
  } dateTime;

  //! size of this member illust
  struct {
    int width,
        height;
  } size;

  //! Linked list of tags
  struct slist tags;

  //!Image link
  char *image;
};

/*!Loads Meta data for illustration id
 * @param account Account to use to download it
 * @param illust  Illustration to load with metadata
 * @param id      illustration id to load */
int pixivtool_illust_init( struct pixivtool_account *account,
                           struct pixivtool_illust  *illust,
                           unsigned int id);

/*!Downloads file from illustration meta data
 * @param account acount to use to download with
 * @param illust  illustartion metadata to use */
int pixivtool_illust_dl( struct pixivtool_account *account,
                         struct pixivtool_illust  *illust );

/*!Cleans up meta data for illustration
 * @param illust the illustation to clean */
int pixivtool_illust_cleanup( struct pixivtool_illust  *illust );

