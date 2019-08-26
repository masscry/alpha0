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

#include "json2/j2dynstr.h"
#include "json2/j2value.h"
#include "json2/j2parse.h"
#include "json2/j2print.h"

#endif