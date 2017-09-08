/**
 * @file j2string.c
 * @author masscry
 * @date 31.08.2017
 *
 * j2string routines
 */

#pragma once
#ifndef __J2_STRING_C__
#define __J2_STRING_C__

J2VAL j2InitString(const char* str) {
  size_t len = strlen(str);
  J2VAL result = (J2VAL)malloc(sizeof(struct _j2_value_));
  if (result == 0) {
    return 0;
  }

  if (len < J2_VALUE_DATA_LEN) {
    result->type = J2_SSTRING;
    memcpy(result->data, str, len+1);
  } else {
    j2String temp = (j2String)malloc(len+1);
    if (temp == 0) {
      free(result);
      return 0;
    }
    result->type = J2_STRING;
    memcpy(temp, str, len+1);
    memcpy(result->data, &temp, sizeof(j2String));
  }
  return result;
}

const char* j2ValueString(const J2VAL val) {
  if (val == 0) {
    return 0;
  }

  switch(val->type) {
  case J2_STRING:
    return *((char**)val->data);
  case J2_SSTRING:
    return val->data;
  default:
    return 0;
  }
}

#endif