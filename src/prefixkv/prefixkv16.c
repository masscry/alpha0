#include <stdlib.h>
#include <string.h>
#include <prefixkv.h>
#include "prfpriv.h"


PKV_COMPARE_FUNC_SIGNATURE(BitCountCompare16, pa, pb, word)
  uint16_t ka = bkbits16(pkvPairKey((const pkvPair_t*)pa)[(size_t)word]);
  uint16_t kb = bkbits16(pkvPairKey((const pkvPair_t*)pb)[(size_t)word]);
  return ka - kb;  
PKV_COMPARE_FUNC_END()
 
/**
 * Sort using 64-bit key.
 */
PKV_COMPARE_FUNC_SIGNATURE(pkvComparePairKey, pa, pb, keylen) 
  const uint16_t* ka = pkvPairKey((const pkvPair_t*)pa);
  const uint16_t* kb = pkvPairKey((const pkvPair_t*)pb);
  size_t i;

  for (i = 0; i < (size_t)keylen; ++i)
  {
    int delta = kb[i] - ka[i];
    if (delta != 0)
    {
      return delta;
    }
  }
  return 0;
PKV_COMPARE_FUNC_END()

// This function is same for 8-bit and 16-bit trees
void pkvCleanupBranch(pkvBranch_t branch);

// Build single branch
static pkvBranch_t pkvBuildBranch16(pkvPair_t* pairs, size_t pairs_cnt, size_t keylen, size_t fullKeyLen) {
  pkvBranch_t result; 

  result = (pkvBranch_t) calloc(1,sizeof(union _pkvBranch_t_));
  if (result == 0) { // No memory error
    return 0;
  }

  result->splt.level = keylen; // Level defined by left key length
  if (keylen == 0) 
  { // If key length is zero
    pkvSort(pairs, pairs_cnt, PKV_PAIR_SIZE(fullKeyLen), pkvComparePairKey, (void*)fullKeyLen); // sort elements in tree by 64-bit value
    result->leaf.data = pairs;     // and save it 
    result->leaf.dlen = pairs_cnt; // to the leaf
  } 
  else 
  { // Otherwise
    pkvPair_t* first;
    pkvPair_t* last;
    
    pkvSort(pairs, pairs_cnt, PKV_PAIR_SIZE(fullKeyLen), BitCountCompare16, (void*) (keylen-1)); // Sort elements by bit count in sub-key.

    first = pairs;
    last = pkvPairArrayItem(pairs, pairs_cnt-1, fullKeyLen);

    if (bkbits16(pkvPairKey(first)[keylen-1]) == bkbits16(pkvPairKey(last)[keylen-1])) 
    {
      // number of bits in first and last element are equal
      // so this branch won't work.
      // we can do optimization here
      // and just drop this level and go deeper.
      free(result); 
      return pkvBuildBranch16(pairs, pairs_cnt, keylen-1, fullKeyLen);
    }
    else
    {
      pkvPair_t* item = pkvPairArrayItem(pairs, pairs_cnt/2, fullKeyLen);

      result->splt.split = bkbits16(pkvPairKey(item)[keylen-1]); // Split sorted element tree in middle
      result->splt.child[0] = pkvBuildBranch16(pairs, pairs_cnt/2, keylen-1, fullKeyLen); // Build left branch
      if (result->splt.child[0] == NULL) 
      {
        free(result); // no memory error
        return 0;
      }

      item = pkvPairArrayItem(pairs, pairs_cnt/2, fullKeyLen);
      result->splt.child[1] = pkvBuildBranch16(item, pairs_cnt - pairs_cnt/2, keylen-1, fullKeyLen); // Build right branch
      if (result->splt.child[1] == 0) 
      {
        pkvCleanupBranch(result->splt.child[0]); // free left branch memory
        free(result); // no memory error
        return 0;
      }
    }
  }
  return result;
}

pkvTree_t pkvBuild16(const pkvPair_t* pairs, size_t pairs_cnt, size_t keylen) 
{
  pkvTree_t result = 0;
  pkvPair_t* tdata = 0;
  pkvBranch_t branch = 0;

  if ((pairs == NULL) || (pairs_cnt == 0) || (keylen == 0)) 
  {
    return NULL; // invalid argument error
  }

  result = (pkvTree_t) malloc(sizeof(struct _pkvTree_t_));
  if (result == NULL) 
  {
    return NULL; // no memory error
  }

  tdata = pkvPairNewArray(keylen, pairs_cnt);
  if (tdata == NULL) 
  {
    return NULL; // no memory error
  }

  memcpy(tdata, pairs, PKV_PAIR_SIZE(keylen)*pairs_cnt); // store copy of data

  branch = pkvBuildBranch16(tdata, pairs_cnt, keylen, keylen); // build root branch
  if (branch == NULL) 
  { // build failed
    free(tdata);
    free(result);
    return 0;
  }

  result->root = branch;
  result->data = tdata;
  result->keylen = keylen;
  return result;
}

/**
 * Binary search in sorted array of value smaller than bound.
 */
static INLINE const pkvPair_t* pkvSearchLess(const pkvPair_t* adata, size_t alen, uint16_t* pBound, size_t keylen) 
{
  size_t first = 0;
  size_t last = alen;
  const pkvPair_t* item;

  assert(adata);
  assert(alen);

  item = pkvPairArrayItemConst(adata, 0, keylen);
  if (pkvPairMaskKeyCheck(item, pBound, NULL, keylen) < 0) 
  { // first element larger than bound
    return adata;
  }

  item = pkvPairArrayItemConst(adata, last-1, keylen);
  if (pkvPairMaskKeyCheck(item, pBound, NULL, keylen) > 0) 
  { // last element smaller than bound
    return 0; // We won't find our key here
  }

  while (first < last) 
  {
    size_t mid = first + (last - first) / 2;
    item = pkvPairArrayItemConst(adata, mid, keylen);

    if (pkvPairMaskKeyCheck(item, pBound, NULL, keylen) <= 0) 
    {
      last = mid;
    } 
    else 
    {
      first = mid + 1;
    }
  }
  return pkvPairArrayItemConst(adata, last - 1, keylen);
}

/**
 * Binary search in sorted array of value greater than bound.
 */
static INLINE const pkvPair_t* pkvSearchGreater(const pkvPair_t* adata, size_t alen, uint16_t* pBound, size_t keylen) {
  size_t first = 0;
  size_t last = alen;
  const pkvPair_t* item;

  assert(adata);
  assert(alen);

  item = pkvPairArrayItemConst(adata, 0, keylen);
  if (pkvPairMaskKeyCheck(item, pBound, NULL, keylen) < 0) 
  { // first element larger than bound
    return 0; // We won't find our key here
  }

  item = pkvPairArrayItemConst(adata, last-1, keylen);
  if (pkvPairMaskKeyCheck(item, pBound, NULL, keylen) > 0) 
  { // last element smaller than bound
    return pkvPairArrayItemConst(adata, last, keylen);
  }

  while (first < last) 
  {
    size_t mid = first + (last - first) / 2;
    item = pkvPairArrayItemConst(adata, mid, keylen);

    if (pkvPairMaskKeyCheck(item, pBound, NULL, keylen) <= 0) 
    {
      last = mid;
    }
    else
    {
      first = mid + 1;
    }
  }
  return pkvPairArrayItemConst(adata, last, keylen);
}

const pkvPair_t* pkvSearchBranch16(const pkvBranch_t branch, const uint16_t* key, const uint16_t* mask, size_t fullKeyLen) 
{
  uint16_t keylen = branch->splt.level; // we know which sub-key to use from tree level

  if (keylen == 0) 
  { // this is leaf
    size_t i;
    pkvLeaf_t bt = branch->leaf;
    const pkvPair_t* lbnd;
    const pkvPair_t* rbnd;

    //
    // Here is main algorithm idea:
    //
    // 1. The key we are search for can't has smaller value, than 
    // his masked version. (so we know the lower bound for our search)
    // XYZW & 0100 = 0Y00
    //
    // 2. The key we are search for also can't be greater, than
    // his value with all masked bits turned on.
    // (so we know the upper bound for our search)
    // 0Y00 | 1011 = 1Y11
    //
    // 0Y00 <= XYZW <= 1Y11
    // 

    uint16_t* boundKey = (uint16_t*) malloc(sizeof(uint16_t)*fullKeyLen);
    for (i = 0; i < fullKeyLen; ++i)
    {
      boundKey[i] = mask[i] & key[i];
    }
    lbnd = pkvSearchLess(bt.data, bt.dlen, boundKey, fullKeyLen);
    
    for (i = 0; i < fullKeyLen; ++i)
    {
      boundKey[i] |= (~key[i]);
    }
    rbnd = pkvSearchGreater(bt.data, bt.dlen, boundKey, fullKeyLen);

    free(boundKey);
    boundKey = NULL;
    
    const pkvPair_t* k;

    if ((lbnd == 0) || (rbnd == 0)) 
    {
      // this can happen if our key can't fit in this bounds
      return NULL;
    }
    
    for (k = lbnd; k != rbnd; k = pkvPairNextConst(k, fullKeyLen)) 
    {
      // linear search from left to right
      if (pkvPairMaskKeyCheck(k, key, mask, fullKeyLen) == 0) 
      {
        return k;
      }
    }

    return NULL;
  } 
  else 
  { // this is splitter
    const pkvPair_t* result = NULL;

    //
    // Search in tree is simple:
    // 1. We know minimum number of set bits - bits(key & mask)
    // 2. We know maximum number of set bits - bits(key & mask) + bits(~mask)
    //
    int low = bkbits16(key[keylen - 1] & mask[keylen - 1]);
    int high = low + bkbits16(~mask[keylen-1]);

    // number of set bits less than splitted, so
    // we need to search only in left subtree
    if (high < branch->splt.split) 
    {
      return pkvSearchBranch16(branch->splt.child[0], key, mask, fullKeyLen);
    }

    // number of set bits greater than splitted, so
    // wee need to search only in right subtree
    if (low > branch->splt.split) 
    {
      return pkvSearchBranch16(branch->splt.child[1], key, mask, fullKeyLen);
    }

    // number of set bits are equal to splitted, so
    // we need to search both subtrees
    result = pkvSearchBranch16(branch->splt.child[0], key, mask, fullKeyLen);
    if (result != 0) 
    {
      return result;
    }
    result = pkvSearchBranch16(branch->splt.child[1], key, mask, fullKeyLen);
    if (result != 0) 
    {
      return result;
    }
  }
  return NULL;
}

const pkvPair_t* pkvSearch16(const pkvTree_t tree, const uint16_t* key,  const uint16_t* mask) 
{
  if ((tree == NULL) || (key == NULL) || (mask == NULL)) 
  {
    return 0;
  }
  return pkvSearchBranch16(tree->root, key, mask, tree->keylen);
}
