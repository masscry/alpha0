#include <stdio.h>
#include <CuTest.h>
#include <string.h>

#include <json2.h>

void TestPrintNull(CuTest *tc) {
    J2VAL root;
    
    root = j2InitNull();    
    char result[100];
    size_t written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 4, written);
    CuAssertIntEquals(tc, 0, strcmp("null", result));
    j2Cleanup(&root);

    root = j2InitTrue();
    written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 4, written);
    CuAssertIntEquals(tc, 0, strcmp("true", result));
    j2Cleanup(&root);

    root = j2InitFalse();
    written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 5, written);
    CuAssertIntEquals(tc, 0, strcmp("false", result));
    j2Cleanup(&root);

    root = j2InitNumber(123.0);
    written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 3, written);
    CuAssertIntEquals(tc, 0, strcmp("123", result));
    j2Cleanup(&root);

    root = j2InitString("root-string");
    written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 13, written);
    CuAssertIntEquals(tc, 0, strcmp("\"root-string\"", result));
    j2Cleanup(&root);

    root = j2InitArray();
    written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 3, written);
    CuAssertIntEquals(tc, 0, strcmp("[\n]", result));
    j2Cleanup(&root);

    root = j2InitObject();
    written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 3, written);
    CuAssertIntEquals(tc, 0, strcmp("{\n}", result));
    j2Cleanup(&root);

}

void TestPrintArrays(CuTest* tc){
    J2VAL root;
    char result[100];

    root = j2InitArray();

    J2VAL item = j2InitNumber(1);
    j2ValueArrayAppend(root, item);

    size_t written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 7, written);
    CuAssertIntEquals(tc, 0, strcmp("[\n  1\n]", result));

    J2VAL item2 = j2InitString("t");
    j2ValueArrayAppend(root, item2);

    written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 14, written);
    CuAssertIntEquals(tc, 0, strcmp("[\n  1,\n  \"t\"\n]", result));

    J2VAL item3 = j2InitObject();
    j2ValueArrayAppend(root, item3);

    written = j2PrintBuffer(root, result, 100);
    CuAssertIntEquals(tc, 23, written);
    CuAssertIntEquals(tc, 0, strcmp("[\n  1,\n  \"t\",\n  {\n  }\n]", result));
    
    j2Cleanup(&root);
    
}

void TestPrintObjects(CuTest* tc){
    J2VAL root;
    char result[100];

    root = j2InitObject();
    J2VAL num = 0;
    J2VAL str = 0;
    J2VAL bl = 0;
    J2VAL arr = 0;
    J2VAL item = 0;

    num = j2InitNumber(555.0);
    str = j2InitString("Text");
    bl = j2InitFalse();
    arr = j2InitArray();
    item = j2InitObject();
    j2ValueArrayAppend(arr, item);
    
    j2ValueObjectItemSet(root, "n", num);
    j2ValueObjectItemSet(root, "s", str);
    j2ValueObjectItemSet(root, "a", arr);
    j2ValueObjectItemSet(root, "b", bl);

    size_t written = j2PrintBuffer(root, result, 100);
    
    CuAssertIntEquals(tc, 79, written);
    
    /*
    // Not very good test, order of items can be changed 
    // almost randomly
    CuAssertIntEquals(tc, 0,
                      strcmp("{\n"
                             "  \"b\": false,\n"
                             "  \"n\": 555,\n"
                             "  \"s\": \"Text\",\n"
                             "  \"a\":\n"
                             "    [\n"
                             "      {\n"
                             "      }\n"
                             "    ]\n"
                             "}" , result));
    */
    j2Cleanup(&root);
    
}

CuSuite *PrinterRegisterTests() {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestPrintNull);
    SUITE_ADD_TEST(suite, TestPrintArrays);
    SUITE_ADD_TEST(suite, TestPrintObjects);
    return suite;
}