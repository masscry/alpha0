#include <assert.h>
#include <stdint.h>

#ifdef _MSC_VER
#define INLINE 
#else
#define INLINE inline
#endif

typedef union _pkvBranch_t_* pkvBranch_t;

/**
 * Prefix key tree leaf.
 */
typedef struct pkvLeaf_t {
  pkvPair_t* data;  // array of key-value pairs        (32-bit)
  size_t     dlen;  // size of key-value pairs array   (32-bit)
  uint16_t   level; // level in tree (zero for leaves) (16-bit)
  uint16_t   split; // number of active bits in center (16-bit)
                    // of key-value pairs array
} pkvLeaf_t; // (96-bits)

/**
 * Prefix tree splitting branch.
 */
typedef struct pkvSplit_t {
  pkvBranch_t  child[2]; // two pointer to branches         (64-bit)
  uint16_t     level;    // level in tree (non-zero)        (16-bit)
  uint16_t     split;    // number of active bits in center (16-bit)
                         // of key-value pairs array
} pkvSplit_t; // (96-bits)

/**
 * Prefix tree branch super-class.
 */
union _pkvBranch_t_ {
  pkvLeaf_t   leaf; // can be leaf
  pkvSplit_t  splt; // or can be split
};

/**
 * Prefix tree root.
 */
struct _pkvTree_t_ {
  pkvBranch_t     root; ///< Tree root
  pkvPair_t*      data; ///< Pointer to key-value array.
  size_t        keylen; ///< Tree full key length
};

// This algorithm uses total set bit count of each
// byte/short of key to sort kv-array
//
// Functions to count set bits are heavily used,
// so I tested several ways to do this.

//#define INTRSIC_WAY 
#ifdef INTRSIC_WAY
// This approach uses standard(?) function "population count"
//
// __builtin_popcount - gcc specific 
// __popcnt - vs specific
// 
// Processor with SSE4 built-in support (popcnt instruction).
//
static INLINE uint16_t bkbits16(uint16_t value) {
  return __builtin_popcount(value);
}
static INLINE uint8_t bkbits8(uint8_t value) {
  return __builtin_popcount(value);
}
#else
#ifdef BK_WAY
//
// Basic algorithm from Brian Kernighan book.
//
static INLINE uint16_t bkbits(uint16_t value) {
  uint16_t c;
  for (c = 0; value; c++) {
    value &= value - 1;
  }
  return c;
}
static INLINE uint8_t bkbits(uint8_t value) {
  uint8_t c;
  for (c = 0; value; c++) {
    value &= value - 1;
  }
  return c;
}
#else
//
// Table based algorithm.
//
static const unsigned char BitsSetTable256[256] = 
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};
static INLINE uint16_t bkbits16(uint16_t value) {
  return BitsSetTable256[value & 0xFF] + BitsSetTable256[(value >> 8)&0xFF];
}

static INLINE uint8_t bkbits8(uint8_t value) {
  return BitsSetTable256[value & 0xFF];
}
#endif
#endif
