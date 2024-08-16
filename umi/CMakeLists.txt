cmake_minimum_required(VERSION 3.20)

project(umi LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 23)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(cmake/FindLLVM.cmake)
    find_package(LLVM REQUIRED HINTS "${LLVM_CMAKE_DIR}")
endif()

if(NOT DEFINED LLVM_COMMON_CMAKE_UTILS)
    set(LLVM_COMMON_CMAKE_UTILS ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
endif()

include(${LLVM_COMMON_CMAKE_UTILS}/Modules/CMakePolicy.cmake NO_POLICY_SCOPE)

list(APPEND CMAKE_MODULE_PATH "${LLVM_DIR}")

#if (APPLE)
#    include(FindLLVM)
#endif ()
#
#find_package(LLVM REQUIRED CONFIG)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(LLVM_LIBS core)

include_directories(include)

add_library(umi-unicode)
target_sources(
    umi-unicode PRIVATE
    include/unicode/utf8.h
    include/unicode/utf8/algorithm.h
    include/unicode/utf8/code_point.h
    include/unicode/utf8/iterator.h
    include/unicode/utf8/string.h
    include/unicode/utf8/string_view.h
    lib/unicode/utf8/code_point.h
    lib/unicode/utf8/algorithm.cpp
    lib/unicode/utf8/code_point.cpp
    lib/unicode/utf8/iterator.cpp
    lib/unicode/utf8/string.cpp
    lib/unicode/utf8/string_view.cpp
    lib/unicode/utf8/sso.h
    lib/unicode/utf8/sso.cpp
)
target_include_directories(umi-unicode PUBLIC include)

add_library(umi-algo INTERFACE include/algorithm/algo.h)
target_include_directories(umi-algo INTERFACE include)

add_library(umi-support)
target_sources(
        umi-support PRIVATE
        include/umi/assert.h
        include/umi/types/vector.h
        include/umi/types/map.h
        include/umi/types/string.h
        include/umi/types/set.h
        include/algorithm/ssize.h

        lib/support/id.cpp
        lib/support/parallel.cpp
)
target_link_libraries(umi-support PUBLIC umi-unicode umi-algo)

add_library(umi-parse)
target_sources(
        umi-parse PRIVATE
        #include/umi/parse/lexer.h
        include/umi/parse/parser.h

        #lib/parse/lexer.cpp
        lib/parse/parser.cpp
        lib/parse/parse_rd.cpp
)
target_link_libraries(umi-parse PUBLIC umi-support)

add_executable(umi-concurrency_check tutorial/threadful.cpp)

configure_file(examples/fib.umi test.py COPYONLY)
configure_file(examples/main.umi test.py COPYONLY)

add_executable(umi-parser_tblgen tools/parser_tblgen.cpp)
set_target_properties(umi-parser_tblgen PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools
)

add_executable(umi-token_dump tools/token_dump.cpp)
set_target_properties(umi-token_dump PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tools
)
target_link_libraries(umi-token_dump PRIVATE umi-parse)


option(ENABLE_TESTING "Build tests (requires Catch2)" OFF)
if (ENABLE_TESTING)
    include(CTest)
    find_package(Catch2 3 REQUIRED)

    add_executable(simple_lex_test test/lex.test.cpp)
    target_link_libraries(simple_lex_test PRIVATE Catch2::Catch2WithMain umi-parse)

    add_executable(simple_parse_test test/parse.test.cpp)
    target_link_libraries(simple_parse_test PRIVATE Catch2::Catch2WithMain umi-parse)

    add_executable(partition_range_test test/partition_range.test.cpp)
    target_link_libraries(partition_range_test PRIVATE Catch2::Catch2WithMain algo)

    add_executable(llvm_trial test/llvm_trial.test.cpp)
    target_include_directories(llvm_trial PRIVATE ${LLVM_INCLUDE_DIRS})
    target_compile_definitions(llvm_trial PRIVATE ${LLVM_DEFINITIONS})
    target_link_libraries(llvm_trial PRIVATE Catch2::Catch2WithMain ${LLVM_LIBS})
    set_target_properties(llvm_trial PROPERTIES CMAKE_CXX_STANDARD 23)
endif()

add_executable(parse_dreams test/parse.test.cpp)
#target_link_libraries(parse_dreams PRIVATE umi-parse)
