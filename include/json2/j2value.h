/**
 * @file j2tree.h
 * @author masscry
 * @date 31.08.2017
 *
 * Standard C JSON Parser Tree.
 *
 */

#pragma once
#ifndef __J2_TREE_HEADER__
#define __J2_TREE_HEADER__

#include <stdint.h>
#include <udict.h>

/**
 * J2 Value Types.
 */
enum _j2_type_ {
  J2_UNDEF   = 0,
  J2_STRING  = 1, /**< Simple single-byte (!non-UTF8) string */
  J2_NUMBER  = 2, /**< Standard number (double) */
  J2_OBJECT  = 3, /**< Hash table (udict) */
  J2_ARRAY   = 4, /**< Simple array */
  J2_TRUE    = 5, /**< True */
  J2_FALSE   = 6, /**< False */
  J2_NULL    = 7, /**< Null */
  J2_LAST    = 8, /**< Last type */
  /* EXTENDED TYPE (CAN'T BE RETURNED BY j2Type) */
  J2_SSTRING = 9, /**< Short string <8 chars == J2_STRING  */
  J2_SARRAY = 10  /**< Single element array(can be useful)(NOT IMPLEMENTED) */
};
struct _j2_value_;
/**
 * Value item.
 */
typedef struct _j2_value_* J2VAL;

/**
 * Pack string into value.
 *
 * @param str string to pack
 * @return packed value, or zero on error
 */
J2API J2VAL j2InitString(const char* str);

/**
 * Pack number into value.
 *
 * @param val value to pack
 * @return packed value, or zero on error
 */
J2API J2VAL j2InitNumber(double val);

/**
 * Create new object-value.
 * @return packed value, or zero on error
 */
J2API J2VAL j2InitObject();

/**
 * Create new array-value.
 * @return packed value, or zero on error
 */
J2API J2VAL j2InitArray();

/**
 * Pack true into value.
 * @return packed value, or zero on error
 */
J2API J2VAL j2InitTrue();

/**
 * Pack false into value.
 * @return packed value, or zero on error
 */
J2API J2VAL j2InitFalse();

/**
 * Pack null into value.
 * @return packed value, or zero on error
 */
J2API J2VAL j2InitNull();

/**
 * Cleanup j2value memory.
 *
 * @param val active value
 */
J2API void j2Cleanup(J2VAL* val);

/**
 * Get value type.
 *
 * @param val valid value
 * @return value type
 * @see _j2_type_
 */
J2API int j2Type(const J2VAL val);

/**
 * Get packed value as string.
 *
 * @param val valid value
 * @return packed value
 */
J2API const char* j2ValueString(const J2VAL val);

/**
 * Get packed value as number.
 *
 * @param val valid value
 * @return packed value
 */
J2API double j2ValueNumber(const J2VAL val);

/**
 * Get object items count.
 * 
 * @param val valid object
 * @return object item count
 */
J2API uint32_t j2ValueObjectSize(const J2VAL val);

/**
 * Get object-value by key.
 *
 * @param obj valid value
 * @param key string
 * @return packed value
 */
J2API J2VAL j2ValueObjectItem(const J2VAL obj, const char* key);

/**
 * Set object member by key.
 *
 * @param obj valid J2VAL object
 * @param key string
 * @param value value to add to object
 */
J2API int j2ValueObjectItemSet(J2VAL obj, const char* key, J2VAL value);

/**
 * Get first item in object.
 * 
 * @param obj valid object
 * @return iterator or zero
 */
J2API UDITEM j2ValueObjectIterFirst(J2VAL obj);

/**
 * Get next item in object.
 * 
 * @param obj valid object
 * @param iter current place in iterator
 * @return next iterator or zero
 */
J2API UDITEM j2ValueObjectIterNext(J2VAL obj, UDITEM iter);

/**
 * Get key from iterator.
 * 
 * @param item valid iterator
 * @return iterator key, or zero
 */
J2API const char* j2ValueObjectIterKey(UDITEM iter);
/**
 * Get value from iterator
 * 
 * @param iter valid iterator
 * @return iterator value, or zero
 */
J2API J2VAL j2ValueObjectIterValue(UDITEM iter);

/**
 * Get array-value by index.
 *
 * @param val valid value
 * @param index item index
 * @return packed value
 */
J2API J2VAL j2ValueArrayIndex(const J2VAL val, uint32_t index);

/**
 * Get array-value size.
 *
 * @param val valid value
 * @return array size
 */
J2API uint32_t j2ValueArraySize(const J2VAL val);

/**
 * Append value to array.
 *
 * @param array valid array
 * @param item item to append
 * @return new item index, or -1 on error
 */
J2API int32_t j2ValueArrayAppend(J2VAL array, J2VAL item);

/**
 * Get number from object, or default value.
 *
 * @param jo valid json object 
 * @param name object field name
 * @param defval default value
 * 
 * @return number in object field, or default value
 */
J2API double joGetNumber(J2VAL obj, const char* item, double defval);

/**
 * Get string from object, or default value.
 *
 * @param jo valid json object 
 * @param name object field name
 * @param defval default value
 * 
 * @return string in object field, or default value
 */
J2API const char* joGetString(J2VAL obj, const char* item, const char* defval);

/**
 * Get number from array, or default value.
 *
 * @param ja valid json array
 * @param index number index
 * @param defval default value
 */
J2API double jaGetNumber(J2VAL arr, uint32_t index, double defval);

/**
 * Get string from array, or default value.
 *
 * @param ja valid json array
 * @param index number index
 * @param defval default value
 */
J2API const char* jaGetString(J2VAL arr, uint32_t index, const char* defval);

/**
 * Traverse tree by path.
 *
 * "" - return root
 * ".name" - return name item from object
 * "#id" - return id item from array
 * ".name#id" - return id item from array with name
 */
J2API const J2VAL joFind(const J2VAL root, const char* path);


#endif /* __J2_TREE_HEADER__ */
