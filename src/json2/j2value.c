/**
 * @file j2value.c
 * @author masscry
 * @date 31.08.2017
 *
 * Common J2 value routines.
 *
 */

#include <string.h>

#include <udict.h>
#include <json2.h>

/**
 * Inner j2 key-value struct.
 */
typedef struct _j2_obj_keyval_ {
  J2VAL val;
  char key[];
} *J2OBJKV;

/**
 * Basic dynamic array for j2 array.
 */
typedef struct _dyn_array_ {
  uint32_t size;
  uint32_t cap;
  J2VAL items[];
} *DARR;

/**
 * Initial array capacity
 */
#define DARR_ICAP (7)

/**
 * Initial object capacity
 */
#define DICT_ICAP (8)

typedef char* j2String;
typedef double j2Number;
typedef UDICT j2Object;
typedef DARR j2Array;

/**
 * Size of data stored in J2VAL
 */
#define J2_VALUE_DATA_LEN (8)

/**
 * Hidden J2VAL struct, only library knows actual structure.
 */
struct _j2_value_ {
  uint32_t type; /**< Value type */
  char data[J2_VALUE_DATA_LEN]; /**< Value data */
};

int j2Type(const J2VAL val) {
  if (val == 0) {
    return J2_UNDEF;
  }

  switch(val->type) {
  case J2_SSTRING: // J2_SSTRING - only for inner optimization usage
    return J2_STRING;
  default:
    return val->type;
  }

}

/**
 * We need deep cleanup for object data.
 */
static void j2ObjectCleanupItem(UDITEM item) {
  J2OBJKV keyval = (J2OBJKV) udValue(item);
  j2Cleanup(&keyval->val);
  free(keyval);
}

void j2Cleanup(J2VAL* pval) {
  if ((pval != 0) && (*pval != 0)) {
    J2VAL val = *pval;
    switch(val->type) {
      case J2_STRING:
      {
        char* temp = *((char**)val->data);
        free(temp);
        break;
      }
      case J2_ARRAY:
      {
        uint32_t i;
        DARR darr = *((DARR*)val->data);
        for (i = 0; i < darr->size; ++i) {
          j2Cleanup(darr->items + i);
        }
        free(darr);
        break;
      }
      case J2_OBJECT:
      {
        UDICT dict = *((UDICT*)val->data);
        udCleanupDeep(&dict, j2ObjectCleanupItem);
        break;
      }
    }
    free(val);
    *pval = 0;
  }
}

double joGetNumber(J2VAL obj, const char* item, double defval) {
  J2VAL jitem = j2ValueObjectItem(obj, item);
  if (jitem == 0) {
    return defval;
  }
  if (j2Type(jitem) != J2_NUMBER) {
    return defval;
  }
  return j2ValueNumber(jitem);
}

double jaGetNumber(J2VAL arr, uint32_t index, double defval) {
  J2VAL jitem = j2ValueArrayIndex(arr, index);
  if (jitem == 0) {
    return defval;
  }
  if (j2Type(jitem) != J2_NUMBER) {
    return defval;
  }
  return j2ValueNumber(jitem);
}

const char* joGetString(J2VAL obj, const char* item, const char* defval) {
  J2VAL jitem = j2ValueObjectItem(obj, item);
  if (jitem == 0) {
    return defval;
  }
  if (j2Type(jitem) != J2_STRING) {
    return defval;
  }
  return j2ValueString(jitem);
}

const char* jaGetString(J2VAL arr, uint32_t index, const char* defval) {
  J2VAL jitem = j2ValueArrayIndex(arr, index);
  if (jitem == 0) {
    return defval;
  }
  if (j2Type(jitem) != J2_STRING) {
    return defval;
  }
  return j2ValueString(jitem);
}

const J2VAL joFind(const J2VAL root, const char* cpath) {
  char* cursor;
  char* ncur;
  char* buffer;
  char oldcur;
  J2VAL croot;
  uint32_t len;
  uint32_t index;

  if (strcmp(cpath, "") == 0) {
    return root;
  }
  
  len = strlen(cpath) + 1;
  buffer = (char*) malloc(len);
  memcpy(buffer, cpath, len);
  
  croot = root;
  cursor = buffer;

  while(1) {
    switch (*cursor) {
      case 0:    // return current
        goto CLEANUP;
      case ' ':  // skip spaces
      case '\t': // skip tabs 
      case '\v':
      case '\n':
        ++cursor;
        break;
      case '.':  // search in object
      case '/':  // search in object alias
        ++cursor;
        ncur = strpbrk(cursor, ".#/");
        if (ncur != 0) {
          oldcur = *ncur;
          *ncur = 0;
        }
        if (j2Type(croot) != J2_OBJECT) {
          croot = 0;
          goto CLEANUP;
        }
        croot = j2ValueObjectItem(croot, cursor);
        if (croot == 0) {
          goto CLEANUP;
        }

        if (ncur == 0) {
          goto CLEANUP;
        }
        *ncur = oldcur;
        cursor = ncur;
        break;
      case '#': // search in array
        ++cursor;
        ncur = strpbrk(cursor, ".#/");
        if (ncur != 0) {
          oldcur = *ncur;
          *ncur = 0;
        }
        if (j2Type(croot) != J2_ARRAY) {
          croot = 0;
          goto CLEANUP;
        }

        index = atoi(cursor);
        croot = j2ValueArrayIndex(croot, index);
        if (croot == 0) {
          goto CLEANUP;
        }

        if (ncur == 0) {
          goto CLEANUP;
        }
        *ncur = oldcur;
        cursor = ncur;
        break;
      default:  // unknown pos
        croot = 0;
        goto CLEANUP;
    }
  }

CLEANUP:
  free(buffer);
  return croot;
}

#include "j2value/j2special.c"
#include "j2value/j2number.c"
#include "j2value/j2string.c"
#include "j2value/j2array.c"
#include "j2value/j2object.c"

