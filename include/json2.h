/**
 * @file json2.h
 * @author masscry
 *
 * Single public json printer/parser header
 *
 */

#pragma once
#ifndef __JSON2_HEADER__
#define __JSON2_HEADER__

#ifdef __cplusplus
#define J2API extern "C"
#else
#define J2API
#endif

/**
 * This JSON parser implementation pretend to be living in a world where only
 * single byte encodings exists.
 * 
 * But, standard demands UTF-8 support, so this parser ignores multibyte-symbols
 * where no iconv support presented.
 * 
 * And, convert multibyte characters to single byte in encoding given by
 * JSON_ENCODING_IN_PROGRAM macro
 */

#define JSON_ENCODING_IN_PROGRAM ("CP866")

#include "json2/j2dynstr.h"
#include "json2/j2value.h"
#include "json2/j2parse.h"
#include "json2/j2print.h"

#endif