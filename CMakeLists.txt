cmake_minimum_required(VERSION 3.10)

project (alpha0 LANGUAGES C)

include(GNUInstallDirs)

enable_testing()

set(ALPHA0_SOURCES
    ./src/rbuf.c
    ./src/udict.c
    ./src/json2/j2dynstr.c
    ./src/json2/j2parse.c
    ./src/json2/j2print.c
    ./src/json2/j2value.c
    ./contrib/mur32.c

    ./src/prefixkv/pkvpair.c
    ./src/prefixkv/prefixkv16.c
    ./src/prefixkv/prfpriv.h
    ./src/prefixkv/prefixkv.c
)

set(CMAKE_C_STANDARD 99)

add_library(alpha0 STATIC ${ALPHA0_SOURCES})

if(ALPHA0_CHECK_COVERAGE)
    target_compile_options(alpha0 PUBLIC
        -fprofile-arcs
        -ftest-coverage
    )
endif(ALPHA0_CHECK_COVERAGE)


target_include_directories(alpha0 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS alpha0 EXPORT alpha0-config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ./include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT alpha0-config DESTINATION share/cmake)

export(TARGETS alpha0 FILE alpha0-config.cmake)

add_executable(rbuf-udict-test test/rbuf-udict-test.c)

target_link_libraries(rbuf-udict-test PRIVATE
    alpha0
)

if(ALPHA0_CHECK_COVERAGE)
    target_link_libraries(rbuf-udict-test PRIVATE
        gcov
    )
endif(ALPHA0_CHECK_COVERAGE)


add_test(NAME rbuf-udict-test COMMAND ./rbuf-udict-test)

add_executable(json2-test
    test/json2-test.c
    test/j2parser-test.c
    test/j2printer-test.c
    contrib/CuTest.c)

target_include_directories(json2-test PRIVATE
    ./contrib
)

target_link_libraries(json2-test PRIVATE
    alpha0
)

if(ALPHA0_CHECK_COVERAGE)
    target_link_libraries(json2-test PRIVATE
        gcov
    )
endif(ALPHA0_CHECK_COVERAGE)

add_test(NAME json2-test COMMAND ./json2-test)

add_executable(js-valid
    test/js-valid.c
)

target_link_libraries(js-valid PRIVATE
    alpha0
)

if(ALPHA0_CHECK_COVERAGE)
    target_link_libraries(js-valid PRIVATE
        gcov
    )
endif(ALPHA0_CHECK_COVERAGE)

add_test(NAME js-valid
    COMMAND ./js-valid ${CMAKE_CURRENT_SOURCE_DIR}/share/sample.jsn)

