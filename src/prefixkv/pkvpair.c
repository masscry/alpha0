#include <prefixkv.h>
#include "prfpriv.h"

#include <string.h>
#include <stdlib.h>

struct _pkvPair_t_
{
  const void* value;
  uint16_t key[];
};

const void* pkvPairValue(const pkvPair_t* pair)
{
  if (pair != NULL)
  {
    return pair->value;
  }
  return NULL;
}

void pkvPairSetValue(pkvPair_t* pair, const void* newValue)
{
  if (pair != NULL)
  {
    pair->value = newValue;
  }
}

const uint16_t* pkvPairKey(const pkvPair_t* pair)
{
  if (pair != NULL)
  {
    return pair->key;
  }
  return NULL;
}

void pkvPairSetKey(pkvPair_t* pair, const uint16_t* newKey, size_t keyLen)
{
  if (pair != NULL)
  {
    memcpy(pair->key, newKey, keyLen*sizeof(uint16_t));
  }
}

pkvPair_t* pkvPairNewArray(size_t keyLength, size_t size)
{
  return (pkvPair_t*) calloc(size, sizeof(struct _pkvPair_t_) + sizeof(uint16_t)*keyLength);
}

void pkvPairArrayCleanup(pkvPair_t* pair)
{
  free(pair);
}

pkvPair_t* pkvPairArrayItem(pkvPair_t* pair, size_t index, size_t keyLen)
{
  return (pkvPair_t*)(((uint8_t*)pair) + (sizeof(struct _pkvPair_t_) + sizeof(uint16_t)*keyLen)*index);
}

const pkvPair_t* pkvPairArrayItemConst(const pkvPair_t* pair, size_t index, size_t keyLen)
{
  return (const pkvPair_t*)(((const uint8_t*)pair) + (sizeof(struct _pkvPair_t_) + sizeof(uint16_t)*keyLen)*index);
}

pkvPair_t* pkvPairNext(pkvPair_t* pair, size_t keyLen)
{
  return (pkvPair_t*)(((uint8_t*)pair) + (sizeof(struct _pkvPair_t_) + sizeof(uint16_t)*keyLen));
}

const pkvPair_t* pkvPairNextConst(const pkvPair_t* pair, size_t keyLen)
{
  return (const pkvPair_t*)(((const uint8_t*)pair) + (sizeof(struct _pkvPair_t_) + sizeof(uint16_t)*keyLen));
}

#define MAX_NULL_MASK (32)

const uint16_t nullMask[MAX_NULL_MASK] = 
{
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

int pkvPairMaskKeyCheck(const pkvPair_t* pair, const uint16_t* key, const uint16_t* mask, size_t keylen)
{
  size_t i;

  assert(keylen < MAX_NULL_MASK);

  if (mask == NULL)
  { //
    // Function can be used when all bits are needed, in that case - we are using nullMask, where all bits are set
    //
    mask = nullMask;
  }

  for (i = 0; i < keylen; ++i)
  {
    int keyDelta =  (key[i] & mask[i]) - (pair->key[i] & mask[i]);
    if (keyDelta != 0)
    {
      return keyDelta;
    }
  }
  return 0;
}
