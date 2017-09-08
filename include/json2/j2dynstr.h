/**
 * @file j2dynstr.h
 * @author masscry
 * @date 04.09.2017
 * 
 * Dynamic string routines.
 * 
 */

#pragma once
#ifndef __J2_DYN_STRING_HEADER__
#define __J2_DYN_STRING_HEADER__

#include <stdlib.h>

#define DS_INITIAL_LEN (8)

typedef struct dynstr_t {
    char* buffer;
    size_t len;
    size_t cap;
} dynstr_t;

/**
 * Create new dynamic string.
 * 
 * @return new dynamic string or zero on errors
 */
dynstr_t* dsInit(); 

/**
 * Delete dynamic string created by dsInit.
 * 
 * @param str string to delete
 */
void dsCleanup(dynstr_t* str);

/**
 * Append character to string.
 * 
 * @param str valid dynamic string
 * @param smb character to append
 * 
 * @return non-zero on errors
 */
int dsAppend(dynstr_t* str, char smb);

/**
 * Return string buffer captured by dynamic string,
 * after that string buffer memery must be released by free call.
 * 
 * @param str valid dynamic string
 * 
 * @return captured string (may be zero)
 */
char* dsReleaseBuffer(dynstr_t* str);



#endif /* __J2_DYN_STRING_HEADER__ */