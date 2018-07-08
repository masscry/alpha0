/**
 * @file j2print.c
 * @author masscry
 * @date 06.05.16
 *
 * JSON print routines implementation.
 *
 */

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <json2/j2print.h>

size_t PrintSpaces(write_func_t write, void* context, int indent){
    if (indent > 0) {
        char spacebuffer[100];
        int spacelen = sprintf(spacebuffer, "%*s", indent, "");
        return write(context, spacebuffer, spacelen);
    }
    return 0;
}

size_t j2PrintFunc(write_func_t write, void* context, const J2VAL root, int offset){
    size_t result = 0;

    result += PrintSpaces(write, context, offset);
    switch (j2Type(root)) {
        case J2_TRUE: {
            result += write(context, "true", 4);
            break;
        }
        case J2_FALSE: {
            result += write(context, "false", 5);
            break;
        }
        case J2_NULL: {
            result += write(context, "null", 4);
            break;
        }
        case J2_NUMBER: {
            char buffer[64];
            int len = sprintf(buffer, "%g", j2ValueNumber(root));
            result += write(context, buffer, len);
            break;
        }
        case J2_STRING: {
            result += write(context, "\"", 1);
            result += write(context, j2ValueString(root), strlen(j2ValueString(root)));
            result += write(context, "\"", 1);
            break;
        }
        case J2_ARRAY: {
            result += write(context, "[\n", 2);
            size_t sz = j2ValueArraySize(root);
            if (sz != 0) {
                for (size_t i = 0; i < sz - 1; ++i) {
                    result += j2PrintFunc(write, context, j2ValueArrayIndex(root, i), offset + 2);
                    result += write(context, ",\n", 2);
                }
                result += j2PrintFunc(write, context, j2ValueArrayIndex(root, sz - 1), offset + 2);
                result += write(context, "\n", 1);
            }

            result += PrintSpaces(write, context, offset);
            result += write(context, "]", 1);
            break;
        }
        case J2_OBJECT: {
            result += write(context, "{\n", 2);

            size_t sz = j2ValueObjectSize(root);

            if (sz != 0){
                UDITEM iter = j2ValueObjectIterFirst(root);                
                J2VAL value = j2ValueObjectIterValue(iter);
                const char* key = j2ValueObjectIterKey(iter);

                for (size_t i = 0; i < sz - 1; ++i){

                    result += PrintSpaces(write, context, offset + 2);
                    result += write(context, "\"", 1);                   // "
                    result += write(context, key, strlen(key));          // "key

                    switch(j2Type(value)) {
                        case J2_ARRAY:
                        case J2_OBJECT:
                            result += write(context, "\":\n", 3);                 // "key":
                            result += j2PrintFunc(write, context, value, offset + 4);  // "key": value
                            break;
                        default:
                            result += write(context, "\": ", 3);                 // "key":
                            result += j2PrintFunc(write, context, value, offset);  // "key": value
                    }
                    result += write(context, ",\n", 2);                  // "key": value,

                    iter = j2ValueObjectIterNext(root, iter);
                    value = j2ValueObjectIterValue(iter);
                    key = j2ValueObjectIterKey(iter);

                }

                result += PrintSpaces(write, context, offset + 2);
                result += write(context, "\"", 1);                   // "
                result += write(context, key, strlen(key));          // "key

                switch(j2Type(value)) {
                    case J2_ARRAY:
                    case J2_OBJECT:
                        result += write(context, "\":\n", 3);                 // "key":
                        result += j2PrintFunc(write, context, value, offset + 4);  // "key": value
                        break;
                    default:
                        result += write(context, "\": ", 3);                 // "key":
                        result += j2PrintFunc(write, context, value, offset);  // "key": value
                }
                result += write(context, "\n", 1);                  // "key": value,
            }

            result += PrintSpaces(write, context, offset);
            result += write(context, "}", 1);
            break;
        }
    }
    return result;
}

typedef struct simple_context {
    char* buffer;
    size_t bufsize;
    size_t cursor;
} simple_context;

size_t simple_write_string(void* context, const void* buffer, size_t bufsize){
    simple_context* sc = (simple_context*)context;
    if (sc->bufsize-sc->cursor > bufsize){
        memcpy(sc->buffer + sc->cursor, buffer, bufsize);
        sc->cursor += bufsize;
        return bufsize;
    }
    return 0;
}

size_t j2PrintBuffer(const J2VAL root, void* buffer, size_t bufsize){
    simple_context sc;
    sc.cursor = 0;
    sc.buffer = (char*)buffer;
    sc.bufsize = bufsize;
    size_t result = j2PrintFunc(simple_write_string, &sc, root, 0);
    sc.buffer[result] = 0;
    return result;
}

size_t simple_write_file(void* context, const void* buffer, size_t bufsize) {
    FILE* output = (FILE*) context;
    if (fwrite(buffer, bufsize, 1, output) != 1) {
        return 0;
    }
    return bufsize;
}

size_t j2PrintFile(const J2VAL root, FILE* file) {
    return j2PrintFunc(simple_write_file, file, root, 0);
}