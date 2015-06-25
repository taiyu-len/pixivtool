/*!@file illust_parse.h
 * Parses a char* webpage to extract metadata out of it into illustration */
#pragma once
#include "illust.h"
#include "account.h"

enum
{
  PARSE_SUCCESS,
  PARSE_FAILURE,
  PARSE_MANGA
};


/*!Parses a given member_illust page for meta data
 * @param  illust to load metadata with
 * @param  page   holds the webpage
 * @return PARSE_SUCCESS for single image illustrations,
 *         PARSE_FAILURE for complete failure
 *         PARSE_MANGA   if its a multi image illustrations.*/
int pixivtool_illust_parse( struct pixivtool_illust *illust,
                            const  char             *page );

/*!Parses a multi illustration page. this is used when `pixivtool_illust_parse'
 * returns PARSE_MANGA
 * @param  illust to load metadata with
 * @param  page   to hold webpage
 * @return */
int pixivtool_illust_manga( struct pixivtool_illust *illust,
                            const  char             *page );

