#include <stdlib.h>
#include <string.h>

#include <json2.h>

dynstr_t* dsInit() {
    dynstr_t* result = (dynstr_t*) malloc(sizeof(dynstr_t));
    if (result == 0) {
        return 0;
    }
    result->buffer = (char*) calloc(DS_INITIAL_LEN, 1);
    if (result->buffer == 0) {
        free(result);
        return 0;
    }
    result->len = 0;
    result->cap = DS_INITIAL_LEN;
    return result;
}

char* dsReleaseBuffer(dynstr_t* str) {
    if (str == 0) {
        return 0;
    }
    char* result = str->buffer;
    str->buffer = 0;
    str->cap = 0;
    str->len = 0;
    return result;
}

void dsCleanup(dynstr_t* str) {
    if (str != 0) {
        free(str->buffer);
    }
    free(str);
}

int dsAppend(dynstr_t* str, char smb) {
    if (str == 0) {
        return -1;
    }

    if (str->buffer == 0) {
        str->buffer = (char*) calloc(DS_INITIAL_LEN, 1);
        if (str->buffer == 0) {
            return -1;
        }
        str->cap = DS_INITIAL_LEN;
        str->len = 0;
    }

    if (str->len + 1 == str->cap) {
        char* temp = (char*) calloc(str->cap * 3/2, 1);
        if (temp == 0) {
            return -1;
        }
        memcpy(temp, str->buffer, str->len);
        free(str->buffer);
        str->buffer = temp;
        str->cap = str->cap * 3/2;
    }

    str->buffer[str->len] = smb;
    ++str->len;
    return 0;
}
