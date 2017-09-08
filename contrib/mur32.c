/**
 * @file mur32.c
 *
 * Canonical MurmurHash3 x86 32-bit version, taken from 
 * https://en.wikipedia.org/wiki/MurmurHash
 *
 * @copyright 
 * MurmurHash3 was written by Austin Appleby, and is placed in the public 
 * domain. The author hereby disclaims copyright to this source code.
 *
 */

#include <stdint.h>

uint32_t ChkMurMur3 (const void *buf,uint32_t len,uint32_t seed) 
{
  static const uint32_t c1 = 0xcc9e2d51;
  static const uint32_t c2 = 0x1b873593;
  static const uint32_t r1 = 15;
  static const uint32_t r2 = 13;
  static const uint32_t m = 5;
  static const uint32_t n = 0xe6546b64;
  const uint8_t *tail;
  uint32_t hash = seed,k,k1;
  const int nblocks = len/4;
  const uint32_t *blocks = (const uint32_t *)buf;
  int i;

  for (i = 0; i < nblocks; i++) 
  {
    k = blocks[i];
    k *= c1;
    k = (k << r1) | (k >> (32 - r1));
    k *= c2;
    hash ^= k;
    hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
  }

  tail = (const uint8_t *) (((const uint8_t*)buf) + nblocks * 4);
  k1 = 0;

  switch (len & 3) 
  {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = (k1 << r1) | (k1 >> (32 - r1));
            k1 *= c2;
            hash ^= k1;
  }

  hash ^= len;
  hash ^= (hash >> 16);
  hash *= 0x85ebca6b;
  hash ^= (hash >> 13);
  hash *= 0xc2b2ae35;
  hash ^= (hash >> 16);
  return hash;
}