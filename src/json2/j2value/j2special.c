/**
 * @file j2special.c
 * @author masscry
 * @date 31.08.2017
 *
 * j2true j2false j2null routines
 */

#pragma once
#ifndef __J2_SPECIAL_C__
#define __J2_SPECIAL_C__

static J2VAL j2InitSpecial(int type) {
  J2VAL result = (J2VAL)malloc(sizeof(struct _j2_value_));
  if (result == 0) {
    return 0;
  }
  result->type = type;
  return result;
}

J2VAL j2InitTrue() {
  return j2InitSpecial(J2_TRUE);
}

J2VAL j2InitFalse() {
  return j2InitSpecial(J2_FALSE);
}

J2VAL j2InitNull() {
  return j2InitSpecial(J2_NULL);
}

#endif 