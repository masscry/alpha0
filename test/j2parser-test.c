#include <CuTest.h>
#include <stdio.h>
#include <string.h>

#include <json2.h>

void TestVoid(CuTest *tc) {
    J2VAL number = j2ParseBuffer("", 0);
    CuAssertPtrEquals(tc, 0, number);
}

void TestNumber(CuTest *tc) {
    J2VAL number = j2ParseBuffer("  123", 0);
    CuAssertPtrNotNull(tc, number);
    CuAssert(tc, "Invalid type", j2Type(number) == J2_NUMBER);
    CuAssert(tc, "Invalid value", j2ValueNumber(number) == 123);
    j2Cleanup(&number);

    number = j2ParseBuffer("-123", 0);
    CuAssertPtrNotNull(tc, number);
    CuAssert(tc, "Invalid type", j2Type(number) == J2_NUMBER);
    CuAssert(tc, "Invalid value", j2ValueNumber(number) == -123);
    j2Cleanup(&number);

    number = j2ParseBuffer("0", 0);
    CuAssertPtrNotNull(tc, number);
    CuAssert(tc, "Invalid type", j2Type(number) == J2_NUMBER);
    CuAssert(tc, "Invalid value", j2ValueNumber(number) == 0);
    j2Cleanup(&number);

    number = j2ParseBuffer("0.01", 0);
    CuAssertPtrNotNull(tc, number);
    CuAssert(tc, "Invalid type", j2Type(number) == J2_NUMBER);
    CuAssert(tc, "Invalid value", j2ValueNumber(number) == 0.01);
    j2Cleanup(&number);

    number = j2ParseBuffer("1.5e3", 0);
    CuAssertPtrNotNull(tc, number);
    CuAssert(tc, "Invalid type", j2Type(number) == J2_NUMBER);
    CuAssert(tc, "Invalid value", j2ValueNumber(number) == 1.5e3);
    j2Cleanup(&number);

    number = j2ParseBuffer("1.5e-3", 0);
    CuAssertPtrNotNull(tc, number);
    CuAssert(tc, "Invalid type", j2Type(number) == J2_NUMBER);
    CuAssert(tc, "Invalid value", j2ValueNumber(number) == 1.5e-3);
    j2Cleanup(&number);
}

void TestString(CuTest *tc) {
    J2VAL str = j2ParseBuffer("\"\"", 0); // Empty string
    CuAssertPtrNotNull(tc, str);
    CuAssert(tc, "Invalid type", j2Type(str) == J2_STRING);
    CuAssert(tc, "Invalid value", strcmp(j2ValueString(str), "") == 0);
    j2Cleanup(&str);

    str = j2ParseBuffer("\"text\"", 0); // Simple string
    CuAssertPtrNotNull(tc, str);
    CuAssert(tc, "Invalid type", j2Type(str) == J2_STRING);
    CuAssert(tc, "Invalid value", strcmp(j2ValueString(str), "text") == 0);
    j2Cleanup(&str);

    str = j2ParseBuffer("\"Hello,\\tWorld!\"", 0); // String with escape character
    CuAssertPtrNotNull(tc, str);
    CuAssert(tc, "Invalid type", j2Type(str) == J2_STRING);
    CuAssert(tc, "Invalid value", strcmp(j2ValueString(str), "Hello,\tWorld!") == 0);
    j2Cleanup(&str);

    str = j2ParseBuffer("\"Bad String", 0); // Only open quote
    CuAssertPtrEquals(tc, 0, str);

    str = j2ParseBuffer("Not-a-string", 0); // String without quotes
    CuAssertPtrEquals(tc, 0, str);

}

void TestBoolean(CuTest *tc) {
    J2VAL bl = j2ParseBuffer("true", 0);
    CuAssertPtrNotNull(tc, bl);
    CuAssert(tc, "Invalid type", j2Type(bl) == J2_TRUE);
    j2Cleanup(&bl);

    bl = j2ParseBuffer("false", 0);
    CuAssertPtrNotNull(tc, bl);
    CuAssert(tc, "Invalid type", j2Type(bl) == J2_FALSE);
    j2Cleanup(&bl);

    bl = j2ParseBuffer("fals", 0); // Malformed
    CuAssertPtrEquals(tc, 0, bl);

    bl = j2ParseBuffer("truelove", 0); // Malformed
    CuAssertPtrEquals(tc, 0, bl);

}

void TestNull(CuTest *tc) {

    J2VAL nl = j2ParseBuffer("null", 0);
    CuAssertPtrNotNull(tc, nl);
    CuAssert(tc, "Invalid type", j2Type(nl) == J2_NULL);
    j2Cleanup(&nl);

    nl = j2ParseBuffer("nul", 0); // Malformed
    CuAssertPtrEquals(tc, 0, nl);

    nl = j2ParseBuffer("null2", 0); // Malformed
    CuAssertPtrEquals(tc, 0, nl);

}

void TestArray(CuTest *tc) {

    // Empty array
    J2VAL arr = j2ParseBuffer("[]", 0);
    CuAssertPtrNotNull(tc, arr);
    CuAssert(tc, "Invalid type", j2Type(arr) == J2_ARRAY);
    CuAssertIntEquals(tc, 0, j2ValueArraySize(arr));
    j2Cleanup(&arr);

    // Single element
    arr = j2ParseBuffer("[123]", 0);
    CuAssertPtrNotNull(tc, arr);
    CuAssert(tc, "Invalid type", j2Type(arr) == J2_ARRAY);
    CuAssertIntEquals(tc, 1, j2ValueArraySize(arr));

    J2VAL e0 = j2ValueArrayIndex(arr, 0);
    CuAssertPtrNotNull(tc, e0);
    CuAssert(tc, "Invalid type", j2Type(e0) == J2_NUMBER);
    CuAssert(tc, "Invalid value", j2ValueNumber(e0) == 123);
    j2Cleanup(&arr);

    // Two elements
    arr = j2ParseBuffer("[ -123 , \"test\" ]", 0);
    CuAssertPtrNotNull(tc, arr);
    CuAssert(tc, "Invalid type", j2Type(arr) == J2_ARRAY);
    CuAssertIntEquals(tc, 2, j2ValueArraySize(arr));

    e0 = j2ValueArrayIndex(arr, 0);
    CuAssertPtrNotNull(tc, e0);
    CuAssert(tc, "Invalid type", j2Type(e0) == J2_NUMBER);
    CuAssert(tc, "Invalid value", j2ValueNumber(e0) == -123);

    e0 = j2ValueArrayIndex(arr, 1);
    CuAssertPtrNotNull(tc, e0);
    CuAssert(tc, "Invalid type", j2Type(e0) == J2_STRING);
    CuAssertIntEquals(tc, 0, strcmp("test", j2ValueString(e0)));
    j2Cleanup(&arr);

    // Recursive arrays
    arr = j2ParseBuffer("[[ ],[100]]", 0);
    CuAssertPtrNotNull(tc, arr);
    CuAssert(tc, "Invalid type", j2Type(arr) == J2_ARRAY);
    CuAssertIntEquals(tc, 2, j2ValueArraySize(arr));

    e0 = j2ValueArrayIndex(arr, 0);
    CuAssertPtrNotNull(tc, e0);
    CuAssert(tc, "Invalid type", j2Type(e0) == J2_ARRAY);
    CuAssertIntEquals(tc, 0, j2ValueArraySize(e0));

    e0 = j2ValueArrayIndex(arr, 1);
    CuAssertPtrNotNull(tc, e0);
    CuAssert(tc, "Invalid type", j2Type(e0) == J2_ARRAY);
    CuAssertIntEquals(tc, 1, j2ValueArraySize(e0));

    e0 = j2ValueArrayIndex(e0, 0);
    CuAssertPtrNotNull(tc, e0);
    CuAssert(tc, "Invalid type", j2Type(e0) == J2_NUMBER);
    CuAssertIntEquals(tc, 100, j2ValueNumber(e0));
    j2Cleanup(&arr);

}

void TestObject(CuTest *tc) {
    // Empty object
    J2VAL obj = j2ParseBuffer("{}", 0);
    CuAssertPtrNotNull(tc, obj);
    CuAssertIntEquals(tc, J2_OBJECT, j2Type(obj));
    j2Cleanup(&obj);

    // Malformed
    obj = j2ParseBuffer("{ 0: 100 }", 0);
    CuAssertPtrEquals(tc, 0, obj);

    obj = j2ParseBuffer("{ \"test\": 100 }", 0);
    CuAssertPtrNotNull(tc, obj);
    CuAssertIntEquals(tc, J2_OBJECT, j2Type(obj));

    J2VAL item = j2ValueObjectItem(obj, "test");
    CuAssertPtrNotNull(tc, item);
    CuAssertIntEquals(tc, J2_NUMBER, j2Type(item));
    CuAssertIntEquals(tc, 100, j2ValueNumber(item));
    j2Cleanup(&obj);

    obj = j2ParseBuffer("{ \"test\": 100, \"lol\": \"once\" }", 0);
    CuAssertPtrNotNull(tc, obj);
    CuAssertIntEquals(tc, J2_OBJECT, j2Type(obj));

    item = j2ValueObjectItem(obj, "lol");
    CuAssertPtrNotNull(tc, item);
    CuAssertIntEquals(tc, J2_STRING, j2Type(item));
    CuAssertIntEquals(tc, 0, strcmp("once", j2ValueString(item)));
    j2Cleanup(&obj);

}

CuSuite *j2ParseBufferRegisterTests() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestVoid);
    SUITE_ADD_TEST(suite, TestNumber);
    SUITE_ADD_TEST(suite, TestString);
    SUITE_ADD_TEST(suite, TestBoolean);
    SUITE_ADD_TEST(suite, TestNull);
    SUITE_ADD_TEST(suite, TestArray);
    SUITE_ADD_TEST(suite, TestObject);
    return suite;
}

