/**
 * @file udict.h
 * @author masscry
 *
 * C hash table implementation.
 *
 */

#ifndef __UDICT_HEADER__
#define __UDICT_HEADER__

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _udict_item_;
struct _udict_;

/**
 * Hash table item pointer.
 */
typedef struct _udict_item_* UDITEM;

/**
 * Hash table header pointer.
 */
typedef struct _udict_* UDICT;

/**
 * Create new hash table.
 *
 * @param icap initial capacity
 *
 */
UDICT udInit(uint32_t icap);

/**
 * Create new hash table based on old one, but with differend capacity.
 */
UDICT udRehash(UDICT* old, uint32_t icap);

/**
 * Delete hash table.
 */
void udCleanup(UDICT* ud);

typedef void (*udCleanupFunc)(UDITEM item);

/**
 * Delete hash table with resource cleanup.
 */
void udCleanupDeep(UDICT* ud, udCleanupFunc func);

/**
 * Get table size.
 */
uint32_t udSize(const UDICT ud);

/**
 * Get table capacity
 */
uint32_t udCap(const UDICT ud);

/**
 * Insert data into hash table.
 */
UDITEM udInsert(UDICT ud, uint32_t key, void* data);

/**
 * Insert data into hash table, if such key already exists, replace it.
 */
UDITEM udReset(UDICT ud, uint32_t key, void* data);

/**
 * Find item in hash table.
 */
UDITEM udFind(const UDICT ud, uint32_t key);

/**
 * Get item value from hash table.
 *
 * WARNING: No way to detect that key actually exists.
 *
 */
void* udGet(const UDICT ud, uint32_t key);

/**
 * Get item key.
 */
uint32_t udKey(const UDITEM item);

/**
 * Get item value.
 */
void* udValue(const UDITEM item);

/**
 * Replace item value.
 */
void udSetValue(UDITEM item, void* value);

/**
 * Find next item with equal key.
 */
UDITEM udNext(UDICT ud, UDITEM item);

/**
 * How many items with equal key left.
 */
uint32_t udLeft(UDICT ud, UDITEM item);

/**
 * Get first item in array.
 *  
 * @param ud dict
 * @return first element or zero
 */
UDITEM udIterFirst(UDICT ud);

/**
 * Get next item in array.
 * 
 * @param ud dict
 * @param item current element
 * @return next elemen or zero
 */
UDITEM udIterNext(UDICT ud, UDITEM item);

/**
 * Get last active element in array.
 * 
 * @param ud dict
 * @return last element or zero
 */
UDITEM udIterLast(UDICT ud);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UDICT_HEADER__ */
