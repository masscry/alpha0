/**
 * @file j2print.h
 * @author timur
 * @date 06.05.16.
 */

#pragma once
#ifndef __JSON_PRINTER_HEADER__
#define __JSON_PRINTER_HEADER__

#include <json2/j2value.h>

/**
 * Write callback function type.
 */
typedef size_t (*write_func_t)(void* context, const void* buffer, size_t bufsize);

/**
 * Print json tree using callback function.
 *
 * @param write callback function called each time routine has new data to print
 * @param context context passed as first parameter to callback function write
 * @param root head of json tree to print
 * @param offset initial number of inserted spaces on each printed line
 *
 * @return sum of numbers returned by callback function
 */
size_t j2PrintFunc(write_func_t write, void* context, const J2VAL root, int offset);

/**
 * Print json tree to buffer of defined size.
 *
 * @param root head of json tree to print
 * @param buffer output buffer
 * @param bufsize maximum buffer size
 *
 * @return number of written bytes
 */
size_t j2PrintBuffer(const J2VAL root, void* buffer, size_t bufsize);

#endif // __JSON_PRINTER_HEADER__
