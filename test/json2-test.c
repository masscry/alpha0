#include <json2.h>
#include <CuTest.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#define SHORT_STR ("01234")

#define LONG_STR ("0123456789ABCDEF")

int verbose = 0;

void TestStringPack(CuTest* tc) {
    J2VAL sstr = 0;
    J2VAL str = 0;
    const char* usstr = 0;
    const char* ustr = 0;

    sstr = j2InitString(SHORT_STR);
    CuAssertPtrNotNull(tc, sstr);
    usstr = j2ValueString(sstr);

    CuAssertPtrNotNull(tc, usstr);

    if (usstr == 0) {
        return;
    }

    CuAssertIntEquals(tc, 0, strcmp(SHORT_STR, usstr));


    str = j2InitString(LONG_STR);
    CuAssertPtrNotNull(tc, str);
    ustr = j2ValueString(str);

    CuAssertPtrNotNull(tc, ustr);

    if (ustr == 0) {
        return;
    }

    CuAssertIntEquals(tc, 0, strcmp(LONG_STR, ustr));

    CuAssertIntEquals(tc, J2_STRING, j2Type(str));
    CuAssertIntEquals(tc, J2_STRING, j2Type(sstr));

    j2Cleanup(&str);
    j2Cleanup(&sstr);

    CuAssertTrue(tc, str == 0);
    CuAssertTrue(tc, sstr == 0);
}

void TestNumberPack(CuTest* tc) {
    J2VAL num = j2InitNumber(123.456);
    CuAssertPtrNotNull(tc, num);

    CuAssertIntEquals(tc, J2_NUMBER, j2Type(num));
    CuAssertTrue(tc, j2ValueNumber(num) == 123.456);

    j2Cleanup(&num);

    CuAssertTrue(tc, num == 0);
}

void TestSpecialPack(CuTest* tc) {
    J2VAL t = j2InitTrue();
    J2VAL f = j2InitFalse();
    J2VAL n = j2InitNull();

    CuAssertPtrNotNull(tc, t);
    CuAssertPtrNotNull(tc, f);
    CuAssertPtrNotNull(tc, n);

    CuAssertIntEquals(tc, J2_TRUE, j2Type(t));
    CuAssertIntEquals(tc, J2_FALSE, j2Type(f));
    CuAssertIntEquals(tc, J2_NULL, j2Type(n));

    j2Cleanup(&t);
    j2Cleanup(&f);
    j2Cleanup(&n);

    CuAssertTrue(tc, t == 0);
    CuAssertTrue(tc, f == 0);
    CuAssertTrue(tc, n == 0);
}

void TestArrayPack(CuTest* tc) {
    double test[10];
    J2VAL array = j2InitArray();
    int i = 0;

    CuAssertPtrNotNull(tc, array);

    CuAssertIntEquals(tc, J2_ARRAY, j2Type(array));

    CuAssertIntEquals(tc, 0, j2ValueArraySize(array));

    for (i = 0; i < 10; ++i) {
        test[i] = rand();
        CuAssertIntEquals(tc, i,
                j2ValueArrayAppend(array, j2InitNumber(test[i]))
                );
        CuAssertIntEquals(tc, i + 1, j2ValueArraySize(array));
    }

    CuAssertIntEquals(tc, 10, j2ValueArraySize(array));

    for (i = 0; i < 10; ++i) {
        J2VAL item = j2ValueArrayIndex(array, i);
        CuAssertPtrNotNull(tc, item);
        CuAssertDblEquals(tc, test[i], j2ValueNumber(item), 0.0001);
    }

    j2Cleanup(&array);

    CuAssertTrue(tc, array == 0);

}

void TestObjectPack(CuTest* tc) {
    J2VAL obj = j2InitObject();
    J2VAL dry1 = 0;
    J2VAL dry2 = 0;
    J2VAL ary1 = 0;
    J2VAL ary2 = 0;
    J2VAL test = 0;

    CuAssertPtrNotNull(tc, obj);
    CuAssertIntEquals(tc, J2_OBJECT, j2Type(obj));

    dry1 = j2InitNumber(42.0);
    dry2 = j2InitNumber(43.0);

    ary1 = j2InitArray();
    ary2 = j2InitArray();

    j2ValueArrayAppend(ary1, dry1);
    j2ValueArrayAppend(ary2, dry2);
    j2ValueArrayAppend(ary1, ary2);

    CuAssertIntEquals(tc, 0, j2ValueObjectItemSet(obj, "ary1", ary1));

    test = j2ValueObjectItem(obj, "ary2");
    CuAssertTrue(tc, test == 0);

    test = j2ValueObjectItem(obj, "ary1");
    CuAssertPtrNotNull(tc, test);

    CuAssertIntEquals(tc, J2_ARRAY, j2Type(test));

    test = j2ValueArrayIndex(test, 0);

    CuAssertIntEquals(tc, J2_NUMBER, j2Type(test));
    CuAssertDblEquals(tc, 42.0, j2ValueNumber(test), 0.0001);

    j2Cleanup(&obj);
    CuAssertTrue(tc, obj == 0);
}

#if _WIN32

char optopt = '?';

int getopt(int argc, char* argv[], const char* par) {
    return -1;
}

#endif

CuSuite* PrinterRegisterTests();
CuSuite* j2ParseBufferRegisterTests();

int main(int argc, char* argv[]) {
    int c = 0;
    CuString* output = 0;
    CuSuite* suite = 0;
    CuSuite* suite2 = 0;

    while ((c = getopt(argc, argv, "v")) != -1) {
        switch (c) {
            case 'v':
                ++verbose;
                break;
            case '?':
                fprintf(stderr, "Bad option: -%c\n", optopt);
                break;
        }
    }

    output = CuStringNew();
    suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, TestStringPack);
    SUITE_ADD_TEST(suite, TestNumberPack);
    SUITE_ADD_TEST(suite, TestSpecialPack);
    SUITE_ADD_TEST(suite, TestArrayPack);
    SUITE_ADD_TEST(suite, TestObjectPack);

    suite2 = PrinterRegisterTests();
    CuSuiteAddSuite(suite, suite2);
    CuSuiteDelete(suite2);

    suite2 = j2ParseBufferRegisterTests();
    CuSuiteAddSuite(suite, suite2);
    CuSuiteDelete(suite2);

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);

    printf("%s\n", output->buffer);

    CuStringDelete(output);
    CuSuiteDelete(suite);

#ifdef _WIN32
    system("pause");
#endif

    return 0;
}
