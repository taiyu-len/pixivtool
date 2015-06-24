/* main.c */

#include "account.h"
#include "member_illust.h"
#include "login.h"
#include "debug.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  char *pixiv_id,
       *password;
  char  hidepass[32];
  if(argc != 3 && argc != 2)
  {
    printf("Usage: %s USERNAME PASSWORD\n",argv[0]);
    return 1;
  }

  pixiv_id = argv[1];

  if(argc == 2)
  {
    printf("Please enter password: ");
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);         // get terminal info
    term.c_lflag &= ~(ECHO);                // Turn of echoing
    tcsetattr(STDIN_FILENO, TCSANOW, &term);// set terminal data
    //Read password
    int c, i = 0;
    while((c = getchar()) != '\n' && c != EOF && i < 32)
      hidepass[i++] = c;
    password = hidepass;
    term.c_lflag |= ECHO;                   // Turn on echoing
    tcsetattr(STDIN_FILENO, TCSANOW, &term);// Reset terminal
    putchar('\n');
  }
  else
    password = argv[2];
  struct pixivtool_account       acc;
  struct pixivtool_member_illust illust;
  pixivtool_account_init(&acc);
  if(pixivtool_login  ( &acc, pixiv_id, password ) == LOGIN_SUCCESS)
  {
    print_cookies(acc.session);
    pixivtool_member_illust_init(&acc, &illust, 50483184);
    pixivtool_logout  ( &acc );
  }
}
