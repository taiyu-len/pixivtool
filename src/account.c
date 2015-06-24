/* account.c */

#include "account.h"

void pixivtool_account_init( struct pixivtool_account *account )
{
  //Set initial values
  account->pixiv_id[0] = 0;
  account->session     = NULL;
}

