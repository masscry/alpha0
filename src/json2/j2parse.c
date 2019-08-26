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

#include <json2.h>

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

static void skipSpaces(j2ParseCallback calls, void* context) {
    int cursor = -1;
    while (cursor != 0) {
        cursor = calls.peek(context);
        if (!isspace(cursor)) {
            break;
        }
        calls.get(context);
    }
}

static int expectString(j2ParseCallback calls, void* context, const char* str) {
    size_t len = strlen(str);
    size_t index = 0;

    for (index = 0; index < len; ++index) {
        int cur = calls.peek(context);
        if (cur != str[index]) {
            break;
        }
        calls.get(context);
    }

    if (index != len) {
        return 0;
    }

    return issplit(calls.peek(context));
}

static int extractString(j2ParseCallback calls, void* context, char** pstr) {
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

    calls.get(context);

    while ((chr = calls.peek(context)) != 0) {
        switch (chr) {
            case -1:
            case 0:
                free(dsReleaseBuffer(&result));
                return -1;
            case '\"': // closing quote
                dsAppend(&result, 0);
                *pstr = dsReleaseBuffer(&result);
                calls.get(context);
                return 0;
            case '\\': // Escape character
                calls.get(context);
                chr = calls.peek(context);

                switch (chr) {
                    case '\"': // Quote
                        dsAppend(&result, '\"');
                        calls.get(context);
                        break;
                    case '\\': // Reverse
                        dsAppend(&result, '\\');
                        calls.get(context);
                        break;
                    case '/': // Solidus
                        dsAppend(&result, '/');
                        calls.get(context);
                        break;
                    case 'b': // Backspace
                        dsAppend(&result, '\b');
                        calls.get(context);
                        break;
                    case 'f': // Formfeed
                        dsAppend(&result, '\f');
                        calls.get(context);
                        break;
                    case 'n': // Newline
                        dsAppend(&result, '\n');
                        calls.get(context);
                        break;
                    case 'r': // carriage return
                        dsAppend(&result, '\r');
                        calls.get(context);
                        break;
                    case 't': // tab
                        dsAppend(&result, '\t');
                        calls.get(context);
                        break;
                    case 'u':
                    { // Unicode
#pragma message ("WARNING: Current implementation ignores \\uXXXX")
                        char buffer[5] = {0};
                        char *bufptr = buffer;
                        calls.get(context);
                        for (size_t i = 0; i < 4; ++i) {
                            *bufptr = calls.peek(context);
                            if ((!isxdigit(*((unsigned char *) bufptr))) || (*bufptr == 0)) {
                                return -1;
                            }
                            ++bufptr;
                            calls.get(context);
                        }
                        dsAppend(&result, '?');
                        break;
                    }
                }

                break;
            default: // Any other character
                if (dsAppend(&result, chr) != 0) {
                    return -1;
                }
                calls.get(context);
        }
    }

    free(dsReleaseBuffer(&result));

    // reach end of string without closing quote mark (")
    return -1;
}

static int extractNumber(j2ParseCallback calls, void* context, double* presult) {
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
                calls.get(context);
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

static J2VAL j2ParseFuncSTD(j2ParseCallback calls, void* context) {
    int chr = -1;

    while (chr != 0) {
        chr = calls.peek(context);

        switch (chr) {
            case ' ': // Skip space characters
            case '\t':
            case '\n':
            case '\v':
            case '\f':
            case '\r':
                calls.get(context);
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
                if (extractNumber(calls, context, &val) == 0) {
                    return j2InitNumber(val);
                }
                return 0;
            }
            case '\"':
            {
                char* str = 0;
                if (extractString(calls, context, &str) == 0) {
                    J2VAL result = j2InitString(str);
                    free(str);
                    return result;
                }
                return 0;
            }
            case 't':
            {
                if (expectString(calls, context, "true") == 0) {
                    return 0;
                }
                return j2InitTrue();
            }
            case 'f':
            {
                if (expectString(calls, context, "false") == 0) {
                    return 0;
                }
                return j2InitFalse();
            }
            case 'n':
            {
                if (expectString(calls, context, "null") == 0) {
                    return 0;
                }
                return j2InitNull();
            }
            case '[':
            {
                int expectComma = 0;
                J2VAL result = j2InitArray();
                if (result == 0) {
                    return 0;
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
                                j2Cleanup(&result);;
                                return 0;
                            }
                            calls.get(context);
                            expectComma = 0;
                            /* FALLTHROUGH */
                        default:
                        {
                            if (expectComma != 0) {
                                j2Cleanup(&result);;
                                return 0;
                            }
                            J2VAL item = j2ParseFuncSTD(calls, context);
                            if (item != 0) {
                                if (j2ValueArrayAppend(result, item) < 0) {
                                    j2Cleanup(&result);;
                                    return 0;
                                }
                            }
                            skipSpaces(calls, context);
                            expectComma = 1;
                        }
                    }
                }
                j2Cleanup(&result);;
                break;
            }
            case '{':
            {
                int expectComma = 0;
                J2VAL result = j2InitObject();
                if (result == 0) {
                    return 0;
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
                                j2Cleanup(&result);;
                                return 0;
                            }
                            calls.get(context);
                            expectComma = 0;
                            /* FALLTHROIGH */
                        default:
                        {
                            if (expectComma != 0) {
                                j2Cleanup(&result);;
                                return 0;
                            }
                            J2VAL key = j2ParseFuncSTD(calls, context);
                            if ((key == 0) || (j2Type(key) != J2_STRING)) {
                                j2Cleanup(&key);
                                j2Cleanup(&result);
                                return 0;
                            }
                            skipSpaces(calls, context);
                            if (calls.peek(context) != ':') {
                                j2Cleanup(&key);
                                j2Cleanup(&result);;
                                return 0;
                            }
                            calls.get(context);

                            J2VAL vl = j2ParseFuncSTD(calls, context);
                            if (vl == 0) {
                                j2Cleanup(&key);
                                j2Cleanup(&result);;
                                return 0;
                            }

                            if (j2ValueObjectItemSet(result, j2ValueString(key), vl) != 0) {
                                j2Cleanup(&key);
                                j2Cleanup(&vl);
                                j2Cleanup(&result);;
                                return 0;
                            }

                            j2Cleanup(&key);
                            skipSpaces(calls, context);
                            expectComma = 1;
                        }
                    }
                }
            }
            default:
                return 0;
        }
    }
    return 0;
}

J2VAL j2ParseFunc(j2ParseCallback calls, void* context) {
    J2VAL result = 0;

    char localeName[64];
    strncpy(localeName, setlocale(LC_NUMERIC, 0), 64);
    localeName[63] = 0;

    result = j2ParseFuncSTD(calls, context);

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

static j2ParseCallback stringParse = {j2GetCharStringContext, j2PeekCharStringContext};

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

static j2ParseCallback fileParse = {
  basicGetCharFunc, /**< Return character, iterate to next */
  basicPeekCharFunc /**< Return character, do not iterate to next */
};

J2VAL j2ParseFile(FILE* stream) {
    return j2ParseFunc(fileParse, stream);
}
