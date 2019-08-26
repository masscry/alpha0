#include "udict.h"
#include "rbuf.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>

uintptr_t expects = 0;

double GetTime(struct timespec* start){
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  if (now.tv_nsec > start->tv_nsec){
    return (now.tv_sec - start->tv_sec)
      + (now.tv_nsec - start->tv_nsec)*1.0e-9;
  } else {
    return (now.tv_sec - start->tv_sec - 1)
      + (1e9 + now.tv_nsec - start->tv_nsec)*1.0e-9;
  }
}

#define EXPECT(cond) \
  if (!(cond)) {\
    fprintf(stderr, "Error: %s at %s:%d\n", #cond, __FILE__, __LINE__);\
    exit(-1);\
  } else {\
    fprintf(stderr, "OK: %s at %s:%d\n", #cond, __FILE__, __LINE__);\
    ++expects;\
  }

#define SEXPECT(cond) \
  if (!(cond)) {\
    fprintf(stderr, "Error: %s at %s:%d\n", #cond, __FILE__, __LINE__);\
    exit(-1);\
  } else {\
    ++expects;\
  }

#define RUN(test) \
  {\
    printf("\n=== Test %s ===\n", #test);\
    fflush(stdout);\
    struct timespec start;\
    clock_gettime(CLOCK_MONOTONIC, &start);\
    (test)();\
    printf("done %"PRIuPTR" in %f sec\n", expects, GetTime(&start));\
    expects = 0;\
  }

void t000(){ // Initialization tests
  EXPECT(udInit(0) == 0); // Assume that programmer knows what he wants, so no default initialization witout a hint

  UDICT ud = udInit(100);
  EXPECT(ud != 0); // Now we happily creates a dict.

  EXPECT(udSize(0) == 0); // Return 0 on zero pointer
  EXPECT(udSize(ud) == 0); // No elements in hash table

  EXPECT(udCap(0) == 0); // Return 0 on zero pointer
  EXPECT(udCap(ud) == 100); // Expect 100 elements were allocated

  udCleanup(0); // No segfault here

  udCleanup(&ud);
  EXPECT(ud == 0); // Expect cleanup to zero pointer
}

void t001(){ // Simple inserts
  UDICT ud = udInit(100);

  UDITEM it0 = udInsert(ud, 0, (void*) 1);
  EXPECT(it0 != 0); // Expect udInsert retuns non zero pointer on success
  EXPECT(udSize(ud) == 1); // Expect size grow by 1
  EXPECT(udGet(ud, 0) == (void*) 1); // Expect 1 at 0
  EXPECT(udKey(it0) == 0);
  EXPECT(udValue(it0) == (void*) 1);

  UDITEM it1 = udInsert(ud, 1, (void*) 2);
  EXPECT(it1 != 0); // Expect udInsert retuns non zero pointer on success
  EXPECT(udSize(ud) == 2); // Expect size grow by 1
  EXPECT(udGet(ud, 0) == (void*) 1); // Expect 1 at 0
  EXPECT(udGet(ud, 1) == (void*) 2); // Expect 2 at 1
  EXPECT(udKey(it1) == 1);
  EXPECT(udValue(it1) == (void*) 2);

  UDITEM it2 = udInsert(ud, 2, (void*) 3);
  EXPECT(it2 != 0); // Expect udInsert retuns non zero pointer on success
  EXPECT(udSize(ud) == 3); // Expect size grow by 1
  EXPECT(udGet(ud, 0) == (void*) 1); // Expect 1 at 0
  EXPECT(udGet(ud, 1) == (void*) 2); // Expect 2 at 1
  EXPECT(udGet(ud, 2) == (void*) 3); // Expect 3 at 2
  EXPECT(udKey(it2) == 2);
  EXPECT(udValue(it2) == (void*) 3);

  UDITEM itFFFFFFFF = udInsert(ud, 0xFFFFFFFF, (void*) 112);
  EXPECT(itFFFFFFFF != 0);
  EXPECT(udSize(ud) == 4);
  EXPECT(udGet(ud, 0) == (void*) 1); // Expect 1 at 0
  EXPECT(udGet(ud, 1) == (void*) 2); // Expect 2 at 1
  EXPECT(udGet(ud, 2) == (void*) 3); // Expect 3 at 2
  EXPECT(udGet(ud, 0xFFFFFFFF) == (void*) 112); // Expect 3 at 2
  EXPECT(udKey(itFFFFFFFF) == 0xFFFFFFFF);
  EXPECT(udValue(itFFFFFFFF) == (void*) 112);

  UDITEM it95 = udInsert(ud, 95, (void*) 96);
  EXPECT(it95 != 0);
  EXPECT(udSize(ud) == 5);
  EXPECT(udGet(ud, 0) == (void*) 1); // Expect 1 at 0
  EXPECT(udGet(ud, 1) == (void*) 2); // Expect 2 at 1
  EXPECT(udGet(ud, 2) == (void*) 3); // Expect 3 at 2
  EXPECT(udGet(ud, 0xFFFFFFFF) == (void*) 112); // Expect 3 at 2
  EXPECT(udGet(ud, 95) == (void*) 96); // Expect 3 at 2
  EXPECT(udKey(it95) == 95);
  EXPECT(udValue(it95) == (void*) 96);

  UDITEM test = 0;
  EXPECT((test = udFind(ud, 0)) != 0);
  EXPECT(udKey(test) == 0);
  EXPECT(udValue(test) == (void*) 1);

  EXPECT((test = udFind(ud, 1)) != 0);
  EXPECT(udKey(test) == 1);
  EXPECT(udValue(test) == (void*) 2);

  EXPECT((test = udFind(ud, 2)) != 0);
  EXPECT(udKey(test) == 2);
  EXPECT(udValue(test) == (void*) 3);

  EXPECT((test = udFind(ud, 0xFFFFFFFF)) != 0);
  EXPECT(udKey(test) == 0xFFFFFFFF);
  EXPECT(udValue(test) == (void*) 112);

  EXPECT((test = udFind(ud, 95)) != 0);
  EXPECT(udKey(test) == 95);
  EXPECT(udValue(test) == (void*) 96);

  udCleanup(&ud);
}

void t002(){ // Inserts near capacity
  UDICT ud = udInit(5);
  for (uintptr_t key = 0; key < 5; ++key){
    udInsert(ud, key, (void*) (key + 1));
  }
  for (uintptr_t key = 0; key < 5; ++key){
    EXPECT(udGet(ud, key) == (void*) (key + 1));
  }

  // Simple insert after cap reached must return 0
  EXPECT(udInsert(ud, 5, (void*) 6) == 0);

  udCleanup(&ud);

  ud = udInit(5);
  for (uintptr_t key = 4; key >= 1; --key){
    udInsert(ud, key, (void*) (key + 1));
  }
  for (uintptr_t key = 4; key >= 1; --key){
    EXPECT(udGet(ud, key) == (void*) (key + 1));
  }

  // Insert when cap is not reached must be successful
  EXPECT(udInsert(ud, 6, (void*)7) != 0);
  EXPECT(udGet(ud, 6) == (void*)7); // Expect correct value

  udCleanup(&ud);
}

#define RTESTLEN (7000)



void t003(){ // Random inserts test
  srand(time(0));

  UDICT ud = udInit(RTESTLEN);

  uintptr_t* keys = (uintptr_t*)calloc(RTESTLEN, sizeof(uintptr_t));
  uintptr_t* vals = (uintptr_t*)calloc(RTESTLEN, sizeof(uintptr_t));

  for (uintptr_t i = 0; i < RTESTLEN; ++i){
    do { // Try to generate unique keys
      keys[i] = rand();
    } while (udFind(ud, keys[i]) != 0);

    vals[i] = rand();
    SEXPECT(udInsert(ud, keys[i], (void*) vals[i]) != 0);
  }

  for (uintptr_t i = 0; i < RTESTLEN; ++i){
    SEXPECT(udGet(ud, keys[i]) == (void*) vals[i])
  }

  free(keys);
  free(vals);

  udCleanup(&ud);
}

void t004(){ // Rehashing test
  UDICT ud = udInit(5);
  for (uintptr_t i = 0; i < 5; ++i){
    udInsert(ud, i, (void*) (i + 10));
  }

  UDICT udtemp = udRehash(0, 10);
  EXPECT(udtemp != 0); // Similar to udInit(10)
  udCleanup(&udtemp);

  EXPECT(udRehash(&ud, 0) == 0); // Inherit udInit behavior
  EXPECT(udRehash(&ud, 1) == 0); // Can't rehash to smaller size

  UDICT ud2 = udRehash(&ud, 10);

  EXPECT(ud == 0); // Old hash deleted
  EXPECT(ud2 != 0); // New hash created

  for (uintptr_t i = 0; i < 5; ++i){
    EXPECT(udGet(ud2, i) == (void*) (i + 10));
  }

  udCleanup(&ud2); // Clean up new hash table
}

void t005() { // Replacing test
  UDICT ud = udInit(5);
  for (uintptr_t i = 0; i < 5; ++i){
    udInsert(ud, i, (void*) (i + 10));
    EXPECT(udGet(ud, i) == (void*)(i + 10));
  }

  EXPECT(udReset(ud, 3, (void*) 55) != 0); // Because can insert
  EXPECT(udGet(ud, 3) == (void*) 55);
  EXPECT(udSize(ud) == 5);

  udCleanup(&ud);

  ud = udInit(5);
  for (uintptr_t i = 0; i < 5; ++i){
    udReset(ud, i, (void*) (i + 10)); // Work as unique key inserter
    EXPECT(udGet(ud, i) == (void*)(i + 10));
  }

  EXPECT(udReset(ud, 100, (void*) (101)) == 0); // Can't insert data with different key, when no capacity left

  udCleanup(&ud);
}

void t006(){ // Equal key iteration
  UDICT ud = udInit(5);
  for (uintptr_t i = 0; i < 5; ++i){
    udInsert(ud, 4, (void*) i);
  }
  UDITEM first = udFind(ud, 4);
  EXPECT(first != 0);
  EXPECT(udValue(first) == (void*) 0);
  EXPECT(udLeft(ud, first) == 4);

  for (uintptr_t i = 1; i < 5; ++i){
    first = udNext(ud, first);
    EXPECT(first != 0);
    EXPECT(udValue(first) == (void*) i);
    EXPECT(udLeft(ud, first) == 4 - i);
  }
  EXPECT(udNext(ud, first) == 0);

  udCleanup(&ud);
}

void t007(){
  RBUF rb = rbufInit(5);

  EXPECT(rbufInit(0) == 0); // Do not allocate zero-sized

  RBITEM it = 0;
  intptr_t value = 0;

  EXPECT(rb != 0);

  EXPECT(rbufCap(0) == 0); // Expect no segfault
  EXPECT(rbufSize(0) == 0); // Expect no segfault

  EXPECT(rbufCap(rb) == 5);
  EXPECT(rbufSize(rb) == 0);

  EXPECT(rbufFront(0) == 0); // Expect no segfault
  EXPECT(rbufBack(0) == 0); // Expect no segfault

  EXPECT(rbufFront(rb) == 0);
  EXPECT(rbufBack(rb) == 0);

  value = rand();

  EXPECT(rbufPushBack(0, (void*) value) == 0); // No segfault

  EXPECT(rbufPushBack(rb, (void*) value) != 0);

  EXPECT(rbufSize(rb) == 1);

  EXPECT(rbufFront(rb) != 0);

  EXPECT(rbufBack(rb) != 0);
  EXPECT(rbufBack(rb) == rbufFront(rb));

  it = rbufBack(rb);

  EXPECT(rbufValue(0) == 0); // No segfault
  EXPECT(rbufValue(it) == (void*)value);

  rbufReset(0); // No segfault

  rbufReset(rb);

  EXPECT(rbufSize(rb) == 0);
  EXPECT(rbufFront(rb) == 0);
  EXPECT(rbufBack(rb) == 0);

  rbufCleanup(0); // Expect no segfault
  rbufCleanup(&rb);

  EXPECT(rb == 0);
}

void t008(){
  RBUF rb = rbufInit(4);
  RBITEM it = 0;
  intptr_t value[5] = {
    rand(),
    rand(),
    rand(),
    rand(),
    rand()
  };

  rbufPushBack(rb, (void*) value[0]);
  EXPECT(rbufSize(rb) == 1);

  rbufPushBack(rb, (void*) value[1]);
  EXPECT(rbufSize(rb) == 2);

  rbufPushBack(rb, (void*) value[2]);
  EXPECT(rbufSize(rb) == 3);

  rbufPushBack(rb, (void*) value[3]);
  EXPECT(rbufSize(rb) == 4);

  rbufPushBack(rb, (void*) value[4]);
  EXPECT(rbufSize(rb) == 4);

  EXPECT(rbufPopFront(0) == 0); // Segfault test
  for (int i = 1; i < 5; ++i){
    EXPECT(rbufPopFront(rb) == (void*) value[i]);
    EXPECT(rbufSize(rb) == (4-i));
  }

  EXPECT(rbufPopFront(rb) == 0);

  rbufCleanup(&rb);
}

struct _rbuf_item_ {
  void* data;
};

void t009(){

  RBUF rb = rbufInit(5);
  RBITEM it = 0;
  intptr_t value[5] = {
    rand(),
    rand(),
    rand(),
    rand(),
    rand()
  };
  int index = 0;

  for (int i = 0; i < 5; ++i){
    it = rbufPushBack(rb, (void*) value[i]);
  }

  EXPECT(rbufSize(rb) == 5);

  it = rbufFront(rb);

  rbufNext(rb, 0); // No segfault

  rbufNext(0, it); // No segfault

  index = 0;

  it = rbufFront(rb);
  while(it != 0){
    EXPECT(rbufValue(it) == (void*)value[index]);
    it = rbufNext(rb, it);
    ++index;
  }
  EXPECT(index == 5);

  it = rbufBack(rb);
  index = 4;
  while(it != 0){
    EXPECT(rbufValue(it) == (void*)value[index]);
    it = rbufPrev(rb, it);
    --index;
  }
  EXPECT(index == -1);

  it = rbufPushBack(rb, (void*) 10); // Move ring
  index = 4;
  while(it != 0){
    it = rbufPrev(rb, it);
    --index;
  }
  EXPECT(index == -1); // Expect same number of items

  rbufCleanup(&rb);
}


int main(int argc, char* argv[]){
  RUN(t000);
  RUN(t001);
  RUN(t002);
  RUN(t003);
  RUN(t004);
  RUN(t005);
  RUN(t006);
  RUN(t007);
  RUN(t008);
  RUN(t009);

  // Need check for udLeft with UDITEM from different hash
  return 0;
}
