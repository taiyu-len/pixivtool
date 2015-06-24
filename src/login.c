/* login.c */

#include "login.h"

#include "account.h"
#include "curlcommon.h"
#include "constants.h"

#include <curl/curl.h> // curl_*
#include <stdio.h>     // perror, fprintf
#include <stdlib.h>    // exit
#include <assert.h>    // assert
#include <string.h>    // strlen

//Used for suppressing output when inputing password
#include <termios.h>
#include <unistd.h>

#define LOGIN_URL  "https://www.secure.pixiv.net/login.php"
#define LOGOUT_URL "www.pixiv.net/logout.php"



//Suppress output
static size_t nowrite( char *, size_t, size_t, void * );

int  pixivtool_login_prompt( struct pixivtool_account *account,
                             const char               *pixiv_id)
{
  assert(account);
  //Request pixiv_id if not given
  char pid [PIXIV_ID_MAXLEN];
  char pass[PASSWORD_MAXLEN];
  //terminal info
  struct termios term;

  if(pixiv_id == NULL)
  {
    pixiv_id = pid;
    printf(
      "Please enter your pixiv id or email used to login\n"
      ":: ");
    int c = 0, i = 0;
    while((c = getchar()) != '\n' && c != EOF && i < PIXIV_ID_MAXLEN)
      pid[i++] = c;
    pid[i] = 0;
    putchar('\n');
    pixiv_id = pid;
  }
  //Get terminal info
  tcgetattr(STDIN_FILENO, &term);

  //Set stdin to not echo input
  term.c_lflag &= ~(ECHO);

  //Update terminal now
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
  printf(
    "Please enter the password for your pixiv account\n"
    ":: ");

  //Read password
  int c = 0, i = 0;
  while((c = getchar()) != '\n' && c != EOF && i < PASSWORD_MAXLEN)
    pass[i++] = c;
  pass[i] = 0;
  putchar('\n');

  //Turn echoing back on
  term.c_lflag |= ECHO;

  //restore terminal
  tcsetattr(STDIN_FILENO, TCSANOW, &term);

  /* Loginto account with this data */
  return pixivtool_login(account, pixiv_id, pass);
}

int  pixivtool_login( struct pixivtool_account *account,
                      const char               *pixiv_id,
                      const char               *password )
{
  //Verify that account is a proper pointer
  assert(account != NULL && pixiv_id != NULL && password != NULL);

  /* Return code */
  int retcode;

  /* Start session */
  CURL *curl = account->session = curl_easy_init();

  /* Check whether request was properly initilized */
  if(!curl)
  {
    perror("Something went wrong with the curl library\n");
    exit  (EXIT_FAILURE);
  }

  /* Create and set Login form */
  struct curl_httppost
    *post = NULL,
    *last = NULL;

  curl_formadd(&post, &last,
      CURLFORM_PTRNAME,     "mode",
      CURLFORM_PTRCONTENTS, "login", CURLFORM_END);

  curl_formadd(&post, &last,
      CURLFORM_PTRNAME,     "pixiv_id",
      CURLFORM_COPYCONTENTS, pixiv_id, CURLFORM_END);

  curl_formadd(&post, &last,
      CURLFORM_PTRNAME,     "pass",
      CURLFORM_COPYCONTENTS, password, CURLFORM_END);

  /* Start cookie session */
  curl_easy_setopt(curl, CURLOPT_COOKIEJAR,     "");

  /* Set URL */
  curl_easy_setopt(curl, CURLOPT_URL,           LOGIN_URL);

  /* Set form data for the request */
  curl_easy_setopt(curl, CURLOPT_HTTPPOST,      post);

  /* Prevent it from writing data anywhere if it fails */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nowrite);

  /* Login with error checking */
  if(common_curl_perform(curl) == COMMON_FAILURE)
    retcode = LOGIN_FAILURE;

  /* Close form */
  curl_formfree(post);

  /* Check for successful login */
  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

//  printf("pixivtool_login()::account login ");
  if(response_code != LOGIN_SUCCESS_CODE)
  {
//    printf("failed (%.*s)\n", PIXIV_ID_MAXLEN, pixiv_id);
//    printf("Returned with response code %ld\n", response_code);
    retcode = LOGIN_FAILURE;
  }
  else
  {
    //TODO Verbose mode maybe?
//    printf("successful (%.*s)\n", PIXIV_ID_MAXLEN, pixiv_id);

    //Copy pixiv id to account data
    strcpy(account->pixiv_id, pixiv_id);

    retcode = LOGIN_SUCCESS;
  }

  /* Reset request, while keeping cookies and connection alive */
  curl_easy_reset(curl);

  return retcode;
}

void pixivtool_logout ( struct pixivtool_account *account )
{
  //Verify that account points to something
  assert(account != NULL);

  CURL *curl = account->session;

  /* Set Request information */
  /* Set url to logout url */
  curl_easy_setopt(curl, CURLOPT_URL, LOGOUT_URL);

  /* Prevent it from writing data anywhere */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nowrite);

  /* Perform it */
  common_curl_perform(curl);

  /* Close connection */
  curl_easy_cleanup(curl);

  //TODO verbose option maybe?
//  printf("account logout successful\n");

  /* Clear account data */
  memset(account, 0, sizeof(*account));
}


static size_t nowrite(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  (void)ptr;
  (void)size;
  (void)userdata;
  return nmemb;
}

