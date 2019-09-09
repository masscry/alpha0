/**
 * @file j2parse.h
 * @author masscry
 * @date 06.05.16
 *
 * JSON parser routines.
 *
 */

#pragma once
#ifndef __JSON_PARSER_HEADER__
#define __JSON_PARSER_HEADER__

#include "j2value.h"
#include <stdio.h>

typedef int (*j2GetCharFunc)(void* context);

typedef int (*j2PeekCharFunc)(void* context);

typedef void (*j2OnErrorFunc)(void* context, int line);

/**
 * Parser callback functions
 */
typedef struct j2ParseCallback {
    j2GetCharFunc get;   /**< Return character, iterate to next */
    j2PeekCharFunc peek; /**< Return character, do not iterate to next */
    j2OnErrorFunc error; /**< Function called on parsing errors */
    void* onErrorData;   /**< Data passed to error function */
} j2ParseCallback;

/**
 * Parse null terminated string to json tree.
 *
 * @param string null terminated string to parse
 * @param endp last not processed character
 *
 * @return parsed tree, or zero on error
 */
J2API J2VAL j2ParseBuffer(const char* string, const char** endp);

/**
 * Parse data get from callbacks to json tree.
 *
 * @param calls callbacks serving characters to parser
 * @param context send to callbacks as first argument
 *
 * @return parsed tree, or zero on error
 */
J2API J2VAL j2ParseFunc(j2ParseCallback calls, void* context);

/**
 * @brief Parse file steam
 * 
 * @param stream file stream
 * @return parsed tree, or zero on error
 */
J2API J2VAL j2ParseFileStreamEx(FILE* stream, j2OnErrorFunc onerror, void* errorData);

#define j2ParseFileStream(INPUT) (j2ParseFileStreamEx(INPUT, 0, 0))

#endif /* __JSON_PARSER_HEADER__ */
