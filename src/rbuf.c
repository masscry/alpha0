#include "rbuf.h"
#include <stdio.h>

struct _rbuf_item_{
  void* data;
};

struct _rbuf_ {
  RBITEM data;
  size_t cap;
  size_t start;
  size_t end;
  size_t size;
  rbufCleanupFunc clean;
};

RBUF rbufInit(size_t icap){
  RBUF result = (RBUF)malloc(sizeof(struct _rbuf_));
  if (result == 0){
    return 0;
  }

  if (icap == 0){
    free(result);
    return 0;
  }

  result->data = (RBITEM)calloc(icap, sizeof(struct _rbuf_item_));
  if (result->data == 0){
    free(result);
    return 0;
  }

  result->cap = icap;
  result->start = 0;
  result->end = 0;
  result->size = 0;
  result->clean = 0;
  return result;
}

void rbufReset(RBUF rb){
  if (rb == 0){
    return;
  }
  rb->start = 0;
  rb->end = 0;
  rb->size = 0;
}

void rbufCleanup(RBUF* rbuf){
  if (rbuf != 0){
    if (*rbuf != 0){
      free((*rbuf)->data);
      free(*rbuf);
      *rbuf = 0;
    }
  }
}

size_t rbufSize(const RBUF rbuf){
  if (rbuf != 0){
    return rbuf->size;
  }
  return 0;
}

size_t rbufCap(const RBUF rbuf){
  if (rbuf != 0){
    return rbuf->cap;
  }
  return 0;
}

RBITEM rbufFront(const RBUF rbuf){
  if (rbuf != 0){
    if (rbuf->size == 0){
      return 0;
    }
    return rbuf->data + rbuf->start;
  }
  return 0;
}

RBITEM rbufBack(const RBUF rbuf){
  if (rbuf != 0){
    if (rbuf->size == 0){
      return 0;
    }
    return rbuf->data + rbuf->end;
  }
  return 0;
}

RBITEM rbufPushBack(RBUF rbuf, void* data){
  RBITEM item = 0;

  if (rbuf == 0){
    return 0;
  }

  if (rbuf->size != 0){ // Special case for zero-sized array, when end and start are equal
    rbuf->end = (rbuf->end + 1) % rbuf->cap;
  }

  if (rbuf->size == rbuf->cap){
    rbuf->start = (rbuf->start+1)%rbuf->cap;
  }else{
    ++rbuf->size;
  }

  item = rbufBack(rbuf);
  if (rbuf->clean != 0) {
    rbuf->clean(item->data);
    item->data = 0;
  }
  item->data = data;
  return item;
}

void* rbufPopFront(RBUF rbuf){
  RBITEM item = 0;
  if (rbuf == 0){
    return 0;
  }

  if (rbuf->size == 0){
    return 0;
  }

  item = rbufFront(rbuf);
  rbuf->start = (rbuf->start+1)%rbuf->cap;
  --rbuf->size;
  return item->data;
}

void* rbufValue(const RBITEM item){
  if (item == 0){
    return 0;
  }
  return item->data;
}

int rbufSetValue(RBITEM item, void* value) {
  if (item == 0) {
    return -1;
  }
  item->data = value;
  return 0;
}

static int rbufIndexInside(const RBUF rbuf, size_t index){
  if (rbuf == 0){
    return -1;
  }
  if (rbuf->size == 0){
    return 0;
  }
  if (rbuf->start < rbuf->end){
    return (index >= rbuf->start) && (index <= rbuf->end);
  }
  return (index >= rbuf->start) || (index <= rbuf->end);
}

RBITEM rbufNext(const RBUF rbuf, RBITEM item){
  int ind = 0;
  int nxt = 0;

  if ((rbuf == 0)||(item == 0)){
    return 0;
  }

  ind = item - rbuf->data;
  if ((rbufIndexInside(rbuf, ind) == 0)||(ind == rbuf->end)){
    return 0;
  }

  nxt = (ind + 1)%rbuf->cap;
  return rbuf->data + nxt;
}

static int modulo(int x, int d){
  return (x%d + d)%d;
}

RBITEM rbufPrev(const RBUF rbuf, RBITEM item){
  int ind = 0;
  int prv = 0;

  if ((rbuf == 0)||(item == 0)){
    return 0;
  }

  ind = item - rbuf->data;
  if ((rbufIndexInside(rbuf, ind) == 0)||(ind == rbuf->start)){
    return 0;
  }
  prv = modulo(ind - 1, rbuf->cap);
  return rbuf->data + prv;
}

rbufCleanupFunc rbufSetCleanupFunc(RBUF rbuf, rbufCleanupFunc func) {
  if (rbuf == 0) {
    return 0;
  }
  rbufCleanupFunc result = rbuf->clean;
  rbuf->clean = func;
  return result;
}