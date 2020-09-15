/**
 * @file prefixkv.h
 * @author timur
 * @date 30.04.2018
 * 
 * Prefix KV tree.
 * 
 */

#pragma once
#ifndef __PREFIX_KV_HEADER__
#define __PREFIX_KV_HEADER__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#define PKVAPI extern "C"
#else
#define PKVAPI
#endif

typedef struct _pkvPair_t_ pkvPair_t;

#define PKV_PAIR_SIZE(KEYLEN) (sizeof(void*) + sizeof(uint16_t)*(KEYLEN))

const void* pkvPairValue(const pkvPair_t* pair);

void pkvPairSetValue(pkvPair_t* pair, const void* newValue);

const uint16_t* pkvPairKey(const pkvPair_t* pair);

int pkvPairMaskKeyCheck(const pkvPair_t* pair, const uint16_t* key, const uint16_t* mask, size_t keylen);

void pkvPairSetKey(pkvPair_t* pair, const uint16_t* newKey, size_t keyLen);

pkvPair_t* pkvPairNewArray(size_t keyLength, size_t size);

void pkvPairArrayCleanup(pkvPair_t* pair);

pkvPair_t* pkvPairArrayItem(pkvPair_t* pair, size_t index, size_t keyLen);

pkvPair_t* pkvPairNext(pkvPair_t* pair, size_t keyLen);

const pkvPair_t* pkvPairArrayItemConst(const pkvPair_t* pair, size_t index, size_t keyLen);

const pkvPair_t* pkvPairNextConst(const pkvPair_t* pair, size_t keyLen);

/**
 * Prefix KV tree PIMPL implementation.
 */
typedef struct _pkvTree_t_* pkvTree_t;

/**
 * Build PKV tree from given kv pair. Using 4 short keys
 * 
 * @param pairs kv pairs array
 * @param pairs_cnt pairs array length
 * @param keylen maximum key length
 * @return zero on error, built tree otherwise.
 */
pkvTree_t pkvBuild16(
  const pkvPair_t* pairs,
  size_t pairs_cnt,
  size_t keylen
);

/**
 * Cleanup PKV tree.
 * 
 * @param tree valid PKV tree
 */
void pkvCleanup(pkvTree_t tree);

/**
 * Search kv-pair in short key tree.
 * 
 * @param tree valid key value tree
 * @param key search key
 * @param mask search mask
 * @param keylen key length
 * @return zero if not found, non zero otherwise
 */
const pkvPair_t* pkvSearch16(
  const pkvTree_t tree, 
  const uint16_t* key,
  const uint16_t* mask
);

#ifdef _WIN32

typedef int (__cdecl *pkvCompareFunc_t)(void *, const void *, const void *);

#define PKV_COMPARE_FUNC_SIGNATURE(NAME, LHS, RHS, CONTEXT) \
  int __cdecl NAME(void* CONTEXT, const void* LHS, const void* RHS) \
  {
  
#define PKV_COMPARE_FUNC_END() }

#endif

#ifdef __DOSX__

// DOSX do not provide proper qsort function with additional argument, so 
// we emulate it here. 

extern void* __DOSX_PKV_CONTEXT_PTR;

typedef int (__cdecl *pkvCompareFunc_t)(const void *, const void *);

#define PKV_COMPARE_FUNC_SIGNATURE(NAME, LHS, RHS, CONTEXT) \
  int __cdecl NAME(const void* LHS, const void* RHS) \
  {\
    void* CONTEXT = __DOSX_PKV_CONTEXT_PTR;
    

#define PKV_COMPARE_FUNC_END() }

#endif

#ifdef __linux__

typedef int (*pkvCompareFunc_t)(const void *, const void *, void *);

#define PKV_COMPARE_FUNC_SIGNATURE(NAME, LHS, RHS, CONTEXT) \
  int NAME(const void* LHS, const void* RHS, void* CONTEXT) \
  {
  
#define PKV_COMPARE_FUNC_END() }

#endif /* __linux__ */

#ifndef PKV_COMPARE_FUNC_SIGNATURE
#error "Each platform must define it's own proper PKV_COMPARE_FUNC_SIGNATURE"
#endif

/**
 * qsort_s/qsort_r wrapper.
 */
void pkvSort(void* base, size_t total, size_t size, pkvCompareFunc_t compare, void* context);

#endif /* __PREFIX_KV_HEADER__ */