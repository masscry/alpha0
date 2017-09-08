/**
 * @file j2number.c
 * @author masscry
 * @date 31.08.2017
 *
 * j2number routines
 */

#pragma once
#ifndef __J2_NUMBER_C__
#define __J2_NUMBER_C__


J2VAL j2InitNumber(double val) {
  J2VAL result = (J2VAL)malloc(sizeof(struct _j2_value_));
  if (result == 0) {
    return 0;
  }
  result->type = J2_NUMBER;
  memcpy(result->data, &val, sizeof(j2Number));
  return result;
}

double j2ValueNumber(const J2VAL val) {
  if (val == 0) {
    return 0.0;
  }
  if (val->type == J2_NUMBER) {
    return *((j2Number*)val->data);
  }
  return 0.0;
}

#endif