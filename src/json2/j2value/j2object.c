/**
 * @file j2object.c
 * @author masscry
 * @date 01.09.2017
 *
 * J2 object routines
 *
 */
#pragma once
#ifndef __J2_OBJECT_C__
#define __J2_OBJECT_C__

#include "udict.h"


J2VAL j2InitObject() {
  UDICT dict = 0;
  J2VAL result = (J2VAL) malloc(sizeof(struct _j2_value_));
  if (result == 0) {
    return 0;
  }

  dict = udInit(DICT_ICAP);
  if (dict == 0) {
    free(result);
    return 0;
  }

  result->type = J2_OBJECT;
  memcpy(result->data, &dict, sizeof(UDICT));
  return result;
}

uint32_t ChkMurMur3(const void* buf, uint32_t len, uint32_t seed);

enum j2DictInsertStatus {
  J2D_ERROR_UPDATE = -2,
  J2D_ERROR = -1,
  J2D_OK = 0,
  J2D_UPDATE = 1,
};

static int j2DictInsertKV(UDICT* pdict, uint32_t keyhash, J2OBJKV keyval) {
  UDICT dict = *pdict;

  UDITEM item = udInsert(dict, keyhash, keyval);
  if (item != 0) { // Everything is good, just returns.
    return J2D_OK;
  }

  // Here we somehow failed to insert item
  if (udSize(dict) == udCap(dict)) {
    // Just exceded capacity, so try to rehash
    dict = udRehash(&dict, udCap(dict) << 1);
    if (dict == 0) { // Everything bad, failed to rehash
      return J2D_ERROR;
    }

    // Here we lost old hashtable and aquired new,
    // so we pass it up.
    *pdict = dict;

    // Try again
    item = udInsert(dict, keyhash, keyval);
    if (item != 0) {     // Success from second try
      return J2D_UPDATE; // This indicates, that we updated
                         // dict ptr
    }

    // Rehashing didn't helped at all,
    // but we still must to replace data
    return J2D_ERROR_UPDATE;
  }
  // If we are here, than something is wrong
  return J2D_ERROR;
}

uint32_t j2ValueObjectSize(const J2VAL obj) {
    UDICT dict = 0;
    
    if (obj == 0) {
        return 0;
    }
    
    if (obj->type != J2_OBJECT) {
        return 0;
    }
    
    dict = *((UDICT*)obj->data);
    
    return udSize(dict);
}

int j2ValueObjectItemSet(J2VAL obj, const char* key, J2VAL value) {
  UDICT dict = 0;
  size_t keylen = 0;
  J2OBJKV keyval = 0;
  J2OBJKV sample = 0;
  uint32_t keyhash = 0;
  UDITEM item = 0;
  uint32_t leftcount = 0;
  uint32_t i = 0;

  if ((obj == 0) || (key == 0) || (value == 0)) {
    goto BAD_END;
  }

  if (obj->type != J2_OBJECT) {
    goto BAD_END;
  }

  dict = *((UDICT*)obj->data);

  // UDICT stores items with uint32_t key,
  // so we need to use some string hashing.
  // Internet says, than MurMur3 is one of the best.
  //
  // There are other options through.
  //
  // Later we must have the actual key, because of
  // hash collisions.
  //
  // I decided to store full key with endiding zero,
  // but calculate keyhash without zero.
  //
  keylen = strlen(key) + 1;
  keyhash = ChkMurMur3(key, keylen - 1, 0);

  keyval = malloc(sizeof(struct _j2_obj_keyval_) + keylen);
  if (keyval == 0) {
    goto BAD_END;
  }

  keyval->val = value;
  memcpy(keyval->key, key, keylen);

  // We have several options on adding new item:
  // (1). There is no such keyhash
  // (2). There is keyhash collision, but different key
  // (3). There are keyhash collision and same key
  //
  // UDICT supports multiple same-key situation, so
  // we store all keys with same hash *hopefully*
  // somewhere nearby, and check every key with udNext,
  // before we can strongly say, that there is no
  // such key in dict.

  item = udFind(dict, keyhash);
  if (item == 0) { // Option (1) Simply insert new item
    goto TRY_INSERT;
  }

  // Here we know, that item != 0, so (2) or (3).

  // Idea is, that number of collisions can be
  // greater than 1, so we need to test them all
  // for (3) case.
  leftcount = udLeft(dict, item);
  for (i = 0; i <= leftcount; ++i) {
    sample = (J2OBJKV) udValue(item);
    if (sample == 0) {
      // Something wrong, in normal conditions
      // this won't happen at all
      goto BAD_END;
    }
    if (memcmp(sample->key, key, keylen) == 0) {
      // Option (3), replace with new
      // and cleanup old value
      udSetValue(item, keyval);
      j2Cleanup(&sample->val);
      free(sample);
      return 0;
    }
    item = udNext(dict, item);
  }

  // Option (2), simple insert
TRY_INSERT:

  switch (j2DictInsertKV(&dict, keyhash, keyval)) {
  case J2D_UPDATE: // Need to update data
    memcpy(obj->data, &dict, sizeof(UDICT));
    /* FALLTHROUGHT */
  case J2D_OK:
    return 0;
  case J2D_ERROR_UPDATE:
    memcpy(obj->data, &dict, sizeof(UDICT));
    /* FALLTHROUGHT */
  case J2D_ERROR:
    goto BAD_END;
  default: // FAILSAFE
    goto BAD_END;
  }

BAD_END:

  free(keyval);
  return -1;
}

J2VAL j2ValueObjectItem(const J2VAL obj, const char* key) {
  UDICT dict = 0;
  size_t keylen = 0;
  uint32_t keyhash = 0;
  UDITEM item = 0;
  uint32_t leftitems = 0;
  uint32_t i = 0;

  if ((obj == 0) || (key == 0)) {
    return 0;
  }

  if (obj->type != J2_OBJECT) {
    return 0;
  }

  dict = *((UDICT*)obj->data);
  keylen = strlen(key) + 1;
  keyhash = ChkMurMur3(key, keylen - 1, 0);

  item = udFind(dict, keyhash);
  if (item == 0) {
    return 0;
  }

  leftitems = udLeft(dict, item);
  for (i = 0; (item != 0) && (i <= leftitems); ++i) {
    J2OBJKV kv = (J2OBJKV) udValue(item);
    if (kv != 0) {
      if (memcmp(kv->key, key, keylen) == 0) {
        return kv->val;
      }
    }
    item = udNext(dict, item);
  }
  return 0;
}

UDITEM j2ValueObjectIterFirst(J2VAL obj) {
  UDICT dict = 0;
  
  if (obj == 0) {
    return 0;            
  }
  
  if (obj->type != J2_OBJECT) {
    return 0;
  }
  
  dict = *((UDICT*)obj->data);
    
  return udIterFirst(dict);    
}

UDITEM j2ValueObjectIterNext(J2VAL obj, UDITEM iter) {    
  UDICT dict = 0;
  
  if (obj == 0) {
    return 0;            
  }
  
  if (obj->type != J2_OBJECT) {
    return 0;
  }
  
  dict = *((UDICT*)obj->data);
  return udIterNext(dict, iter);
}
  
const char* j2ValueObjectIterKey(UDITEM iter) {
    if (iter == 0) {
      return 0;
    }
    J2OBJKV kv = (J2OBJKV) udValue(iter);    
    return kv->key;
}

J2VAL j2ValueObjectIterValue(UDITEM iter) {
    if (iter == 0) {
      return 0;
    }
    J2OBJKV kv = (J2OBJKV) udValue(iter);    
    return kv->val;    
}

#endif
