/* login.c */

#include "login.h"

#include "account.h"
#include "curlcommon.h"

#include <curl/curl.h> // curl_*
#include <stdio.h>     // perror, fprintf
#include <stdlib.h>    // exit
#include <assert.h>    // assert
#include <string.h>    // strlen

#define LOGIN_URL  "https://www.secure.pixiv.net/login.php"
#define LOGOUT_URL "www.pixiv.net/logout.php"


//! Login Success code. The code returned by http server on login success
enum
{
  LOGIN_SUCCESS_CODE = 302
};


//used to prevent output
static size_t nowrite( char *, size_t, size_t, void * );

int  pixivtool_login ( struct pixivtool_account *account,
                      const char               *pixiv_id,
                      const char               *password )
{
  //Verify that account is a proper pointer
  assert(account != NULL);

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

  /*   Set Session data */
  {
    /* Start cookie session */
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR,     "");

    /* Set URL */
    curl_easy_setopt(curl, CURLOPT_URL,           LOGIN_URL);

    /* Set form data for the request */
    curl_easy_setopt(curl, CURLOPT_HTTPPOST,      post);

    /* Prevent it from writing data anywhere if it fails */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nowrite);
  }

  /* Login with error checking */
  if(common_curl_perform(curl) == COMMON_FAILURE)
    retcode = LOGIN_FAILURE;

  /* Close form */
  curl_formfree(post);

  /* Check for successful login */
  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

  printf("Account login ");
  if(response_code != LOGIN_SUCCESS_CODE)
  {
    printf("failed (%.*s)\n", PIXIV_ID_MAXLEN, pixiv_id);
    printf("Returned with response code %ld\n", response_code);
    retcode = LOGIN_FAILURE;
  }
  else
  {
    //TODO Verbose mode maybe?
    printf("successful (%.*s)\n", PIXIV_ID_MAXLEN, pixiv_id);
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
  printf("Account logout successful\n");

  /* Clear account data */
  account->pixiv_id[0] = 0;
  account->session = NULL;
}


static size_t nowrite(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  (void)ptr;
  (void)size;
  (void)userdata;
  return nmemb;
}

