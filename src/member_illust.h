/* member_illust.h */
#pragma once
#include "account.h"

//!Stores meta data for member illustration
struct pixivtool_member_illust
{
  //!ID of the member illust.
  unsigned int id;

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
        minute;
  } dateTime;

  //! size of this member illust
  struct {
    int width,
        height;
  } size;

  //! Linked list of tags
  struct slist {
    struct slist *next;
    char         *string;
  } tags;

  //!Server name
  char *server;
};

/*!Loads Meta data for illustration id
 * @param account Account to use to download it
 * @param illust  Illustration to load with metadata
 * @param id      illustration id to load */
int pixivtool_member_illust_init( struct pixivtool_account       *account,
                                  struct pixivtool_member_illust *illust,
                                  unsigned int id);


