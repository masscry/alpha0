/**
 * @file j2array.c
 * @author masscry
 * @date 01.09.2017
 *
 * j2array routines
 */
 
#pragma once
#ifndef __J2_ARRAY_C__
#define __J2_ARRAY_C__

J2VAL j2InitArray() {
  DARR darr = 0;
  J2VAL result = (J2VAL) malloc(sizeof(struct _j2_value_));
  if (result == 0) {
    return 0;
  }

  result->type = J2_ARRAY;

  darr = malloc(
    sizeof(struct _dyn_array_) + sizeof(J2VAL)*DARR_ICAP);

  if (darr == 0) {
    free(result);
    return 0;
  }

  darr->size = 0;
  darr->cap = DARR_ICAP;
  memset(darr->items, 0, sizeof(J2VAL)*DARR_ICAP);

  memcpy(result->data, &darr, sizeof(DARR));
  return result;
}

J2VAL j2ValueArrayIndex(const J2VAL val, uint32_t index) {
  if (val == 0) {
    return 0;
  }

  if (val->type == J2_ARRAY) {
    DARR darr = *((DARR*)val->data);
    if (index < darr->size) {
      return darr->items[index];
    }
  }
  return 0;
}

uint32_t j2ValueArraySize(const J2VAL val) {
  if (val == 0) {
    return 0;
  }
  if (val->type == J2_ARRAY) {
    DARR darr = *((DARR*)val->data);
    return darr->size;
  }
  return 0;
}

int32_t j2ValueArrayAppend(J2VAL array, J2VAL item) {
  if ((array == 0) || (item == 0)) {
    return -1;
  }

  if (array->type == J2_ARRAY) {
    int32_t result = 0;
    DARR darr = *((DARR*)array->data);

    if (darr->cap == darr->size) {

      DARR new_darr = malloc(
        sizeof(struct _dyn_array_)
        + sizeof(J2VAL)*(darr->cap*3/2)
      );

      if (new_darr == 0) {
        return -1;
      }

      memcpy(new_darr, darr,
        sizeof(struct _dyn_array_) + sizeof(J2VAL)*darr->cap);

      new_darr->cap = darr->cap*3/2;

      memcpy(array->data, &new_darr, sizeof(DARR));
      free(darr);
      darr = new_darr;
    }
    darr->items[darr->size] = item;
    result = (int32_t) darr->size;
    ++darr->size;
    return result;
  }

  return -1;

}

#endif