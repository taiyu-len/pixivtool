/* constants.h */
#pragma once

//!Account and password maximum lengths
enum
{
  PIXIV_ID_MAXLEN = 255,
  PASSWORD_MAXLEN = 32
};

//! HTTP return codes.
enum
{
  LOGIN_SUCCESS_CODE = 302,
  DLOAD_SUCCESS_CODE = 200
};
