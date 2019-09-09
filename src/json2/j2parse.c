/**
 * @file j2parse.c
 * @author masscry
 * @date 06.06.16
 *
 * JSON parser implementation.
 *
 */

#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <malloc.h>
#include <assert.h>

#include <json2.h>

typedef struct loc_t {
    int col;
    int line;
} loc_t;

static int issplit(int symbol) {
    switch (symbol) {
        case 0:
        case '}':
        case ']':
        case ',':
        case ' ':
        case '\t':
        case '\n':
            return 1;
        default:
            return 0;
    }
}

static void skipSpaces(j2ParseCallback calls, loc_t* ploc, void* context) {
    int cursor = -1;
    while (cursor != 0) {
        cursor = calls.peek(context);
        if (!isspace(cursor)) {
            break;
        }
        calls.get(context);
        if (cursor == '\n')
        {
            ploc->col = 1;
            ++ploc->line;
        }
        else
        {
            ++ploc->col;
        }
    }
}

static int expectString(j2ParseCallback calls, struct loc_t* ploc, void* context, const char* str) {
    size_t len = strlen(str);
    size_t index = 0;

    for (index = 0; index < len; ++index) {
        int cur = calls.peek(context);
        if (cur != str[index]) {
            break;
        }
        calls.get(context); ++ploc->col;
    }

    if (index != len) {
        return 0;
    }

    return issplit(calls.peek(context));
}

static int utf8OctetLengthExpected(char smb) {
    if ((smb & 0x80) == 0) {
        // 0XXXXXXX
        return 1;
    }
    if ((smb & 0xC0) == 0x80) {
        // tail detected
        return 0;
    }
    if((smb & 0xE0) == 0xC0) {
        // 110XXXXX 10XXXXXX 10XXXXXX 10XXXXXX
        return 2;
    }
    if((smb & 0xF0) == 0xE0) {
        // 1110XXXX 10XXXXXX 10XXXXXX 10XXXXXX
        return 3;
    }
    if((smb & 0xF8) == 0xF0) {
        // 11110XXX 10XXXXXX 10XXXXXX 10XXXXXX
        return 4;
    }
    // invalid character
    assert(0);
    return -1;
}

#define UTF8_MAX_OCTET_BUFFER (4)

static const uint32_t utf8Mask[UTF8_MAX_OCTET_BUFFER+1] = {
    0x3F, // tail has 6 bits to use   10XXXXXX
    0x7F, // 1-byte has 7 bits to use 0XXXXXXX
    0x1F, // 2-byte has 5 bits to use 110XXXXX
    0x0F, // 3-byte has 4 bits to use 1110XXXX
    0x07  // 4-byte has 3 bits to use 11110XXX
};

static uint32_t utf8ExtractOctets(int octets, j2ParseCallback calls, void* context) {
    uint32_t result = 0;

    // read first octet
    int cursor = calls.get(context);
    if (cursor <= 0) {
        return 0;
    }
    result |= cursor & utf8Mask[octets];

    // read left octets
    --octets;
    while(octets-->0) {
        cursor = calls.get(context);
        result <<= 6;
        result |= cursor & utf8Mask[0];
    }
    return result;
}

#ifdef __unix__

#include <iconv.h>

static int dsXAppend(dynstr_t* str, uint32_t smb) {
    iconv_t code;
    char* smbHead;
    size_t smbLeft;
    char result[1];

    char* resultHead;
    size_t resultLeft;
    size_t size;

    code = iconv_open(JSON_ENCODING_IN_PROGRAM, "WCHAR_T");
    if (code == ((iconv_t) -1)) {
        return -1;
    }

    smbHead = (char*) (&smb);
    smbLeft = 4;

    resultHead = result;
    resultLeft = 1;

    size = iconv(code, &smbHead, &smbLeft, &resultHead, &resultLeft);
    if (size == ((size_t)-1)) {
        iconv_close(code);
        return -1;
    }

    if (dsAppend(str, result[0]) != 0) {
        iconv_close(code);
        return -1;
    }

    iconv_close(code);
    return 0;
}

#else
#pragma message ("WARNING: Current implementation ignores UTF-8 characters in non-unix platforms, replaces with ?")

static int dsXAppend(dynstr_t* str, uint32_t smb) {
    return dsAppend(str, '?');
}

#endif

static int extractEscape(j2ParseCallback calls, loc_t* ploc, void* context, dynstr_t* result) {
    char chr;

    calls.get(context); ++ploc->col; // skip '\\'
    chr = calls.peek(context);       // peek next

    switch (chr) {
        case -1:
        case 0:
            return -1;
        case '\"': // Quote
            if (dsAppend(result, '\"') != 0) {
                return -1;
            }
            calls.get(context); ++ploc->col;
            break;
        case '\\': // Reverse
            if (dsAppend(result, '\\') != 0) {
                return -1;
            }
            calls.get(context); ++ploc->col;
            break;
        case '/': // Solidus
            if (dsAppend(result, '/') != 0) {
                return -1;
            }
            calls.get(context); ++ploc->col;
            break;
        case 'b': // Backspace
            if (dsAppend(result, '\b') != 0) {
                return -1;
            }
            calls.get(context); ++ploc->col;
            break;
        case 'f': // Formfeed
            if (dsAppend(result, '\f') != 0) {
                return -1;
            }
            calls.get(context); ++ploc->col;
            break;
        case 'n': // Newline
            if (dsAppend(result, '\n') != 0) {
                return -1;
            } 
            calls.get(context); ++ploc->col;
            break;
        case 'r': // carriage return
            if (dsAppend(result, '\r') != 0) {
                return -1;
            }
            calls.get(context); ++ploc->col;
            break;
        case 't': // tab
            if (dsAppend(result, '\t') != 0) {
                return -1;
            }
            calls.get(context); ++ploc->col;
            break;
        case 'u':
        { // Unicode
            char buffer[5] = {0};
            char *bufptr = buffer;
            calls.get(context); ++ploc->col;
            for (size_t i = 0; i < 4; ++i) {
                *bufptr = calls.peek(context);
                if ((!isxdigit(*((unsigned char *) bufptr))) || (*bufptr == 0)) {
                    return -1;
                }
                ++bufptr;
                calls.get(context); ++ploc->col;
            }
            if (dsXAppend(result, strtoul(buffer, 0, 16)) != 0) {
                return -1;
            }
            break;
        }
    }
    return 0;
}

static int extractString(j2ParseCallback calls, loc_t* ploc, void* context, char** pstr) {
    int chr = -1;
    dynstr_t result;

    if (pstr == 0) {
        return -1;
    }

    chr = calls.peek(context);
    if (chr != '\"') {
        return -1;
    }

    result.buffer = 0;
    result.len = 0;
    result.cap = 0;

    calls.get(context); ++ploc->col;

    while ((chr = calls.peek(context)) != 0) {
        switch (chr) {
            case -1:
            case 0:
                goto ON_EXTRACT_ERROR;
            case '\"': // closing quote
                if (dsAppend(&result, '\0') != 0) {
                    goto ON_EXTRACT_ERROR;
                }
                *pstr = dsReleaseBuffer(&result);
                calls.get(context); ++ploc->col;
                return 0;
            case '\\': // Escape character
                if (extractEscape(calls, ploc, context, &result) != 0) {
                    goto ON_EXTRACT_ERROR;
                }
                break;
            default: // Any other character
            {
                int octets = -1;
                switch ((octets = utf8OctetLengthExpected(chr)))
                {
                    case 0:
                    default:
                        // this can't happend, because long encoded strings 
                        // are already processed in cases 2-4
                        assert(0);
                        goto ON_EXTRACT_ERROR;
                    case 1:
                        if (dsAppend(&result, chr) != 0) {
                            goto ON_EXTRACT_ERROR;
                        }
                        calls.get(context); ++ploc->col;
                        break;
                    case 2:
                    case 3:
                    case 4:
                    {
                        uint32_t symbol = utf8ExtractOctets(octets, calls, context);
                        if (symbol == 0) {
                            // Decode failed, current implementation can't
                            // get 0 from utf8ExtractOctets
                            goto ON_EXTRACT_ERROR;
                        }
                        if (dsXAppend(&result, symbol) != 0) {
                            goto ON_EXTRACT_ERROR;
                        }
                        ++ploc->col;
                        break;
                    }
                }
            }
        }
    }

    // reach end of string without closing quote mark (")

ON_EXTRACT_ERROR:

    free(dsReleaseBuffer(&result));
    return -1;
}

static int extractNumber(j2ParseCallback calls, loc_t* ploc, void* context, double* presult) {
    int chr = -1;
    dynstr_t result;
    char* temp = 0;
    char* end = 0;

    if (presult == 0) {
        return -1;
    }

    result.buffer = 0;
    result.len = 0;
    result.cap = 0;

    while (chr != 0) {
        chr = calls.peek(context);
        ++ploc->col;
        switch (chr) {
            case '+':
            case '-': // Number
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '.':
            case 'e':
            case 'E':
                dsAppend(&result, chr);
                calls.get(context); ++ploc->col;
                break;
            default:
                chr = 0;
                break;
        }
    }

    temp = dsReleaseBuffer(&result);

    *presult = strtod(temp, &end);
    // here can be some check for end var.
    free(temp);
    return 0;
}

#define J2_RETURN_ERROR(CONTEXT, CALLS, PLOC) \
    if (CALLS.error != 0)\
    {\
        CALLS.error(CONTEXT, PLOC->line, PLOC->col);\
    }\
    return 0;

static J2VAL j2ParseFuncSTD(j2ParseCallback calls, loc_t* ploc, void* context) {
    int chr = -1;

    while (chr != 0) {
        chr = calls.peek(context);
        ++ploc->col;

        switch (chr) {
            case '\n':
                ploc->col = 0;
                ++ploc->line;
            case ' ': // Skip space characters
            case '\t':
            case '\v':
            case '\f':
            case '\r':
                calls.get(context); ++ploc->col;
                break;
            case '-': // Number
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                double val = 0.0;
                if (extractNumber(calls, ploc, context, &val) == 0) {
                    return j2InitNumber(val);
                }
                J2_RETURN_ERROR(context, calls, ploc);
            }
            case '\"':
            {
                char* str = 0;
                if (extractString(calls, ploc, context, &str) == 0) {
                    J2VAL result = j2InitString(str);
                    free(str);
                    return result;
                }
                J2_RETURN_ERROR(context, calls, ploc);
            }
            case 't':
            {
                if (expectString(calls, ploc, context, "true") == 0) {
                    J2_RETURN_ERROR(context, calls, ploc);
                }
                return j2InitTrue();
            }
            case 'f':
            {
                if (expectString(calls, ploc, context, "false") == 0) {
                    J2_RETURN_ERROR(context, calls, ploc);
                }
                return j2InitFalse();
            }
            case 'n':
            {
                if (expectString(calls, ploc, context, "null") == 0) {
                    J2_RETURN_ERROR(context, calls, ploc);
                }
                return j2InitNull();
            }
            case '[':
            {
                int expectComma = 0;
                J2VAL result = j2InitArray();
                if (result == 0) {
                    J2_RETURN_ERROR(context, calls, ploc);
                }

                calls.get(context);
                chr = -1;

                while (chr != 0) {
                    chr = calls.peek(context);
                    switch (chr) {
                        case ']':
                            calls.get(context);
                            return result;
                        case ',':
                            if (expectComma == 0) {
                                j2Cleanup(&result);
                                J2_RETURN_ERROR(context, calls, ploc);
                            }
                            calls.get(context);
                            expectComma = 0;
                            /* FALLTHROUGH */
                        default:
                        {
                            if (expectComma != 0) {
                                j2Cleanup(&result);;
                                J2_RETURN_ERROR(context, calls, ploc);
                            }
                            J2VAL item = j2ParseFuncSTD(calls, ploc, context);
                            if (item != 0) {
                                if (j2ValueArrayAppend(result, item) < 0) {
                                    j2Cleanup(&result);
                                    J2_RETURN_ERROR(context, calls, ploc);
                                }
                            }
                            skipSpaces(calls, ploc, context);
                            expectComma = 1;
                        }
                    }
                }
                j2Cleanup(&result);
                break;
            }
            case '{':
            {
                int expectComma = 0;
                J2VAL result = j2InitObject();
                if (result == 0) {
                    J2_RETURN_ERROR(context, calls, ploc);
                }

                calls.get(context);
                chr = -1;

                while (chr != 0) {
                    chr = calls.peek(context);
                    switch (chr) {
                        case '}':
                            calls.get(context);
                            return result;
                        case ',':
                            if (expectComma == 0) {
                                j2Cleanup(&result);
                                J2_RETURN_ERROR(context, calls, ploc);
                            }
                            calls.get(context);
                            expectComma = 0;
                            /* FALLTHROIGH */
                        default:
                        {
                            J2VAL key = 0;
                            J2VAL vl = 0;

                            if (expectComma != 0) {
                                j2Cleanup(&result);
                                J2_RETURN_ERROR(context, calls, ploc);
                            }
                            
                            key = j2ParseFuncSTD(calls, ploc, context);
                            if ((key == 0) || (j2Type(key) != J2_STRING)) {
                                j2Cleanup(&key);
                                j2Cleanup(&result);
                                J2_RETURN_ERROR(context, calls, ploc);
                            }
                            skipSpaces(calls, ploc, context);
                            if (calls.peek(context) != ':') {
                                j2Cleanup(&key);
                                j2Cleanup(&result);
                                J2_RETURN_ERROR(context, calls, ploc);
                            }
                            calls.get(context);

                            vl = j2ParseFuncSTD(calls, ploc, context);
                            if (vl == 0) {
                                j2Cleanup(&key);
                                j2Cleanup(&result);;
                                J2_RETURN_ERROR(context, calls, ploc);
                            }

                            if (j2ValueObjectItemSet(result, j2ValueString(key), vl) != 0) {
                                j2Cleanup(&key);
                                j2Cleanup(&vl);
                                j2Cleanup(&result);;
                                J2_RETURN_ERROR(context, calls, ploc);
                            }

                            j2Cleanup(&key);
                            skipSpaces(calls, ploc, context);
                            expectComma = 1;
                        }
                    }
                }
            }
            default:
                J2_RETURN_ERROR(context, calls, ploc);
        }
    }
    J2_RETURN_ERROR(context, calls, ploc);
}

J2VAL j2ParseFunc(j2ParseCallback calls, void* context) {
    J2VAL result = 0;
    loc_t loc;

    loc.line = 1;
    loc.col = 1;

    char localeName[64];
    strncpy(localeName, setlocale(LC_NUMERIC, 0), 64);
    localeName[63] = 0;

    setlocale(LC_NUMERIC, "C");
    result = j2ParseFuncSTD(calls, &loc, context);
    setlocale(LC_NUMERIC, localeName);
    return result;
}

typedef struct j2StringContext {
    const char* cursor;
    int done;
} j2StringContext;

int j2GetCharStringContext(void* context) {
    j2StringContext* cnt = (j2StringContext*) context;
    if (cnt->done == 0) {
        int result = *cnt->cursor;
        if (result == 0) {
            cnt->done = 1;
        } else {
            ++cnt->cursor;
        }
        return result;
    }
    return -1;
}

int j2PeekCharStringContext(void* context) {
    j2StringContext* cnt = (j2StringContext*) context;
    if (cnt->done) {
        return -1;
    }
    return *cnt->cursor;
}

static j2ParseCallback stringParse = {
    j2GetCharStringContext, 
    j2PeekCharStringContext,
    0
};

J2VAL j2ParseBuffer(const char* string, const char** endp) {
    j2StringContext context;
    J2VAL result = 0;

    context.cursor = string;
    context.done = 0;

    result = j2ParseFunc(stringParse, &context);

    if (endp != 0) {
        *endp = context.cursor;
    }

    return result;
}

static int basicGetCharFunc(void* pcon) {
  FILE* fc = (FILE*) pcon;
  if ((!feof(fc)) && (!ferror(fc))) {
    int result = fgetc(fc);
    if (result == EOF) {
      return -1;
    }
    return result;
  }
  return -1;
}

static int basicPeekCharFunc(void* pcon) {
  FILE* fc = (FILE*) pcon;
  if ((!feof(fc)) && (!ferror(fc))) {
    int result = fgetc(fc);
    if (result == EOF) {
      return -1;
    }
    ungetc(result, fc);
    return result;
  }
  return -1;
}

J2VAL j2ParseFileStreamEx(FILE* stream, j2OnErrorFunc onerror) {
    j2ParseCallback fileParse = {
        basicGetCharFunc,
        basicPeekCharFunc,
        onerror
    };
    return j2ParseFunc(fileParse, stream);
}
