/*!@file login.h
 * account login.
 * Functions used to login to an account.
 * */
#pragma once
#include "account.h"

enum
{
  LOGIN_SUCCESS,
  LOGIN_FAILURE
};

/*! Used to login to an account and get login data for using most features which
 *  require to be logged in.
 * @param  account  Address of a pixiv account object
 * @param  pixiv_id Your pixiv account name
 * @param  password Your pixiv account password
 * @return LOGIN_SUCCESS or LOGIN_FAILURE
 * */
int pixivtool_login ( struct pixivtool_account *account,
                      const char               *pixiv_id,
                      const char               *password );

/*! Used to logout of an account 
 * @param account Account to be logged out of */
void pixivtool_logout ( struct pixivtool_account *account );

