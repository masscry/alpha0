/**
 * @file rbuf.h
 * @author masscry
 * @date 07.09.2017
 *
 * C ring buffer implementation.
 *
 */

#ifndef __RBUF_HEADER__
#define __RBUF_HEADER__

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif 

/**
 * Ring buffer item.
 */
typedef struct _rbuf_item_* RBITEM;

/**
 * Ring buffer
 */
typedef struct _rbuf_* RBUF;

typedef void (*rbufCleanupFunc)(void* value);

/**
 * Create new ring buffer with defined capacity.
 * @param icap Ring buffer capacity
 * @return new ring buffer or zero on error
 */
RBUF rbufInit(size_t icap);

/**
 * Remove all data from buffer.
 * @param RBUF ring buffer
 */
void rbufReset(RBUF rbuf);

/**
 * Cleanup ring buffer.
 *
 * After this function invocation, pointer to buffer == 0.
 *
 * @param rbuf pointer to ring buffer
 */
void rbufCleanup(RBUF* rbuf);

/**
 * Get actual ring buffer size.
 *
 * @param rbuf ring buffer
 */
size_t rbufSize(const RBUF rbuf);

/**
 * Get ring buffer maximum capacity, before old data discarded
 *
 * @param rbuf ring buffer
 */
size_t rbufCap(const RBUF rbuf);

/**
 * Add element to the end of buffer.
 *
 * @param rbuf ring buffer
 * @param data data to store
 */
RBITEM rbufPushBack(RBUF rbuf, void* data);

/**
 * Pop element from the buffer front.
 *
 * @param rbuf ring buffer
 * @return popped data
 */
void* rbufPopFront(RBUF rbuf);

/**
 * Get first ring buffer element.
 *
 * @param rbuf ring buffer
 * @return item with stored data or zero
 */
RBITEM rbufFront(const RBUF rbuf);

/**
 * Get last ring buffer element.
 *
 * @param rbuf ring buffer
 * @return item with stored data or zero
 */
RBITEM rbufBack(const RBUF rbuf);

/**
 * Get next buffer element.
 *
 * @param rbuf ring buffer
 * @param item current item
 * @return next item or zero if no items left
 */
RBITEM rbufNext(const RBUF rbuf, const RBITEM item);

/**
 * Get previous buffer element.
 *
 * @param rbuf ring buffer
 * @param item current item
 * @return previous item or zero if no items left
 */
RBITEM rbufPrev(const RBUF rbuf, const RBITEM item);

/**
 * Get ring buffer item value.
 *
 * @param item item with stored data
 * @return stored item value
 */
void* rbufValue(const RBITEM item);

/**
 * Set new value for ring buffer item.
 *
 * @param item item with stored data
 * @param value new value to store
 * @return if new value is set returns 0, otherwise -1
 */
int rbufSetValue(RBITEM item, void* value);

/**
 * Set cleanup function.
 *
 * Cleanup function called when buffer start to overwrite old items.
 *
 * @param rbuf ring buffer
 * @param func cleanup function
 * @return old cleanup function
 */
rbufCleanupFunc rbufSetCleanupFunc(RBUF rbuf, rbufCleanupFunc func);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __RBUF_HEADER__ */

