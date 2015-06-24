/*!@file account.h
 * Account struct.
 * Keeps track of account data, and session. Everything is based around this
 * structure
 * */
#pragma once

#include <curl/curl.h>

//!Account and password maximum lengths
enum
{
  PIXIV_ID_MAXLEN = 255,
  PASSWORD_MAXLEN = 32
};


/*! Account structure object. Holds The current session for logged in account */
struct pixivtool_account
{
  //! CURL easy handle. Keeps connection alive from start to finish
  CURL *session;
  //! pixiv id of this account
  char pixiv_id[PIXIV_ID_MAXLEN];
};

/*!Initilizes a pixivtool account structure. Sets account structure data to NULL
 * @param account the account to */
void pixivtool_account_init ( struct pixivtool_account *account );


