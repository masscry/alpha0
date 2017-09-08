#include <stdio.h>

#include <udict.h>

/**
 * Internal structure of hash table item.
 */
struct _udict_item_ {
  uint32_t key; /**< Key */
  void* value; /**< Value */
};

/**
 * Internal structure of hash table
 */
struct _udict_ {
  uint32_t cap; /**< Capacity */
  uint32_t size; /**< Size */
  uint32_t* dups; /**< Number of duplicates */
  uint8_t* active; /**< Is bucket empty */
  UDITEM data; /**< Actual table */
};

UDICT udInit(uint32_t icap){
  UDICT result = 0;
  void* tmpMem = 0;

  if (icap == 0){
    return 0;
  }
  
  result = (UDICT)malloc(sizeof(struct _udict_));
  if (result == 0){
    return 0;
  }

  result->size = 0;
  result->cap = icap;
  
  /*
   * Small memory allocation optimization.
   *
   * Allocate three arrays with as one chunk.
   */
  tmpMem = calloc(icap, sizeof(uint32_t) + sizeof(uint8_t) + sizeof(struct _udict_item_));
  if (tmpMem == 0){
    free(result);
    return 0;
  }
  
  /*
   * Then setup arrays accordingly
   */
  result->dups = (uint32_t*)tmpMem;
  result->active = (uint8_t*)(result->dups + icap);
  result->data = (UDITEM)(result->active + icap);

  return result;
}

UDICT udRehash(UDICT* old, uint32_t icap){
  UDICT nhash = 0;

  if ((old != 0) && (*old != 0)){
    if (icap < udCap(*old)){
      return 0;
    }
  }
  nhash = udInit(icap);

  if (nhash == 0){
    return 0;
  }

  if ((old != 0) && (*old != 0)){

    /* Actual rehashing */
    uint32_t cap = udCap(*old);
    uint32_t ind = 0;
    for (ind = 0; ind < cap; ++ind){
      if ( (*old)->active[ind] != 0 ){
        /* No test on udInsert result, because everything must fit */
        udInsert(
          nhash,
          (*old)->data[ind].key,
          (*old)->data[ind].value
        );
      }
    }

    udCleanup(old);
  }

  return nhash;

}

void udCleanup(UDICT* ud){
  udCleanupDeep(ud, 0);
}

void udCleanupDeep(UDICT* ud, udCleanupFunc func){
  if (ud != 0){
    if (*ud != 0){
    
      if (func != 0) {
        uint32_t index = 0;
        while((udSize(*ud) > 0) && (index < udCap(*ud))) {
          if ((*ud)->active[index] != 0) {
            func((*ud)->data + index);
            (*ud)->active[index] = 0;
            --(*ud)->size;
          }
          ++index;
        }
      }
      
      free((*ud)->dups);
      free(*ud);
      *ud = 0;
    }
  }
}

uint32_t udSize(const UDICT ud){
  if (ud == 0){
    return 0;
  }
  return ud->size;
}

uint32_t udCap(const UDICT ud){
  if (ud == 0){
    return 0;
  }
  return ud->cap;
}

UDITEM udInsert(UDICT ud, uint32_t key, void* data){
  uint32_t plk = key & (ud->cap-1);
  int looped = 2;

  if (ud->size == ud->cap){ /* No space left */
    return 0;
  }

  while(looped-->0) {
    while ((plk < ud->cap)&&(ud->active[plk] != 0)) {
      ud->dups[plk] += (ud->data[plk].key == key); /* better that if-clause? */
      ++plk;
    }

    if (plk != ud->cap){ /* Found empty item */
      break;
    }

    plk = 0; /* rewind to start */
  }

  if (ud->active[plk] != 0){
    /* after looping didn't found empty item */
    return 0;
  }

  ud->size += 1;
  ud->active[plk] = 1;
  ud->data[plk].key = key;
  ud->data[plk].value = data;
  return ud->data+plk;
}

/**
 * @todo Add test for insertion of new pair when capacity reached.
 */
UDITEM udReset(UDICT ud, uint32_t key, void* data){
  uint32_t plk = key & (ud->cap-1);
  int looped = 2;

  while(looped-->0) {

    /* Here may be added the insertion test */

    while ((plk < ud->cap)&&(ud->active[plk] != 0) /* Is active */
      && (ud->data[plk].key != key)){ /* But different key */
      ++plk;
    }

    if (plk != ud->cap){ /* Found empty item */
      break;
    }

    plk = 0; /* rewind to start */
  }

  if ((ud->active[plk] != 0) && (ud->data[plk].key != key)){
    /* after looping didn't found empty item */
    return 0;
  }

  ud->size += (ud->active[plk] == 0);
  ud->active[plk] = 1;
  ud->data[plk].key = key;
  ud->data[plk].value = data;
  return ud->data+plk;
}



UDITEM udFind(const UDICT ud, uint32_t key){
  uint32_t plk = 0;
  int looped = 2;

  if (ud == 0) {
    return 0;
  }

  plk = key & (ud->cap-1);

  while (looped-->0) {
    while ((plk < ud->cap)          /* And less than cap */
      &&   (ud->active[plk] != 0)     /* Active */
      &&   (ud->data[plk].key != key) /* But different key */
      ){ 
      ++plk; /* Iterate */
    }
    if (plk == ud->cap){ /* Hit capacity, rewind */
      plk = 0;
      continue;
    }
    if (ud->active[plk] == 0){ /* If stopped at non-active, than no items with such key left */
      return 0;
    }
    if (ud->data[plk].key == key){
      return ud->data + plk;
    }
  }
  return 0;
}

void* udGet(const UDICT ud, uint32_t key){
  UDITEM it = udFind(ud, key);
  if (it != 0){
    return it->value;
  }
  return 0;
}

uint32_t udKey(const UDITEM item){
  return item->key;
}

void* udValue(const UDITEM item){
  return item->value;
}

void udSetValue(UDITEM item, void* value) {
  item->value = value;
}

UDITEM udNext(UDICT ud, UDITEM item){
  uint32_t left = udLeft(ud, item);
  int looped = 2;
  uint32_t plk = item - ud->data + 1; /* Next item after this */

  if (left == 0){
    return 0;
  }

  while (looped-->0){
    while ((plk < ud->cap)      /* And less than cap */
      && (ud->active[plk] != 0) /* Active */
      && (ud->data[plk].key != udKey(item)) /* But different key */
      ){
      ++plk; /* Iterate */
    }
    if (plk == ud->cap){ /* Hit capacity, rewind */
      plk = 0;
      continue;
    }
    return ud->data + plk;
  }
  return 0;
}

uint32_t udLeft(UDICT ud, UDITEM item){
  uint32_t pos = item - ud->data;
  return ud->dups[pos];
}

UDITEM udIterFirst(UDICT ud) {
    uint32_t index = 0;
    
    if (ud == 0) {
        return 0;
    }
        
    if (ud->size == 0) {
        return 0;
    }
    
    for (index = 0; index < ud->cap; ++index) {
        if (ud->active[index] != 0) {
            return ud->data + index;
        }        
    }
    return 0;
}

UDITEM udIterNext(UDICT ud, UDITEM item) {
    uint32_t index = 0;
    
    if ((ud == 0) || (item == 0)) {
        return 0;
    }
    
    for (index = item - ud->data + 1; index < ud->cap; ++index) {
        if (ud->active[index] != 0) {
            return ud->data + index;
        }
    }
    return 0;
}

UDITEM udIterLast(UDICT ud) {
    int32_t index = 0;
    
    if (ud == 0) {
        return 0;
    }
    
    for (index = ud->cap - 1; index >= 0; --index) {
        if (ud->active[index] != 0) {
            return ud->data + index;
        }
    }    
    return 0;
}