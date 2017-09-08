cmake_minimum_required(VERSION 3.0)

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
)

add_definitions("-std=gnu99")

add_library(alpha0 ${ALPHA0_SOURCES})

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

target_link_libraries(rbuf-udict-test
    alpha0
)

add_test(NAME rbuf-udict-test COMMAND ./rbuf-udict-test)
