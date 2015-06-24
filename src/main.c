/* main.c */

#include "account.h"
#include "illust.h"
#include "login.h"
#include "debug.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  //TODO make this better
  if(argc == 1)
  {
    printf("Usage: %s a b c ...\n"
           "where [a,b,c,...] are illustration ids\n", argv[0]);
    return EXIT_FAILURE;
  }
  struct pixivtool_account acc;
  struct pixivtool_illust  illust;
  unsigned long int        id;
  //Login
  if(pixivtool_login_prompt(&acc, NULL) == LOGIN_FAILURE)
  {
    printf("Failed to login\n");
    return EXIT_FAILURE;
  }
  printf("Successfully logged into account (%.*s)\n",
      PIXIV_ID_MAXLEN, acc.pixiv_id);

  int i = 0;
  while(++i < argc)
  {
    errno = 0;
    id = strtoul(argv[i], NULL, 10);
    if(errno)
    {
      printf("Invalid input::%s",argv[i]);
      return EXIT_FAILURE;
    }
    //Load up illustration metadata
    pixivtool_illust_init   ( &acc, &illust, id);
    //Download illustration data
    pixivtool_illust_dl     ( &acc, &illust );
    //Clean up meta data
    pixivtool_illust_cleanup( &illust );
  }
  pixivtool_logout     ( &acc );
  printf("Done\n");
  return EXIT_SUCCESS;
}
