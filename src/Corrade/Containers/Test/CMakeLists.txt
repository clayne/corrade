#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
#             Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

# IDE folder in VS, Xcode etc. CMake 3.12+, older versions have only the FOLDER
# property that would have to be set on each target separately.
set(CMAKE_FOLDER "Corrade/Containers/Test")

if(CORRADE_TARGET_EMSCRIPTEN OR CORRADE_TARGET_ANDROID)
    set(CONTAINERS_TEST_DIR ".")
else()
    set(CONTAINERS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h)

# In all following corrade_add_test() macros, if a test wants to use
# CorradeUtilityTestLib, it has to link to CorradeTestSuiteTestLib instead.
# Otherwise the implicitly linked CorradeTestSuite would drag in CorradeUtility
# in addition to CorradeUtilityTestLib, leading to ODR violations and making
# ASan builds fail.

corrade_add_test(ContainersAnyReferenceTest AnyReferenceTest.cpp)
corrade_add_test(ContainersArrayTest ArrayTest.cpp)
corrade_add_test(ContainersArrayTupleTest ArrayTupleTest.cpp LIBRARIES CorradeTestSuiteTestLib)
corrade_add_test(ContainersArrayViewTest ArrayViewTest.cpp)
corrade_add_test(ContainersArrayViewStlTest ArrayViewStlTest.cpp)
corrade_add_test(ContainersBigEnumSetTest BigEnumSetTest.cpp)
corrade_add_test(ContainersBitArrayTest BitArrayTest.cpp LIBRARIES CorradeTestSuiteTestLib)
corrade_add_test(ContainersBitArrayViewTest BitArrayViewTest.cpp LIBRARIES CorradeTestSuiteTestLib)
corrade_add_test(ContainersBitArrayViewBenchmark BitArrayViewBenchmark.cpp LIBRARIES CorradeTestSuiteTestLib)
corrade_add_test(ContainersEnumSetTest EnumSetTest.cpp)
corrade_add_test(ContainersFunctionTest FunctionTest.cpp)
corrade_add_test(ContainersFunctionBenchmark FunctionBenchmark.cpp)

corrade_add_test(ContainersGrowableArrayTest GrowableArrayTest.cpp)
if(CORRADE_TARGET_EMSCRIPTEN)
    if(CMAKE_VERSION VERSION_LESS 3.13)
        message(FATAL_ERROR "CMake 3.13+ is required in order to specify Emscripten linker options")
    endif()
    # TODO: somehow, for std::vector the available memory is enough?
    target_link_options(ContainersGrowableArrayTest PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
endif()
corrade_add_test(ContainersGrowableArraySa___FailTest GrowableArraySanitizerFailTest.cpp)
# While Clang and GCC use -fsanitize=whatever, MSVC allows also /fsanitize=,
# so catch both.
if(CMAKE_CXX_FLAGS MATCHES "[-/]fsanitize=address")
    set_tests_properties(ContainersGrowableArraySa___FailTest PROPERTIES
        PASS_REGULAR_EXPRESSION "AddressSanitizer: container-overflow")
endif()

corrade_add_test(ContainersIterableTest IterableTest.cpp LIBRARIES CorradeTestSuiteTestLib)
corrade_add_test(ContainersLinkedListTest LinkedListTest.cpp)
corrade_add_test(ContainersMoveReferenceTest MoveReferenceTest.cpp)
corrade_add_test(ContainersOptionalTest OptionalTest.cpp)
corrade_add_test(ContainersPairTest PairTest.cpp)
corrade_add_test(ContainersPairStlTest PairStlTest.cpp)
corrade_add_test(ContainersPointerTest PointerTest.cpp)
corrade_add_test(ContainersPointerStlTest PointerStlTest.cpp)
corrade_add_test(ContainersRawForwardListTest RawForwardListTest.cpp)
corrade_add_test(ContainersReferenceTest ReferenceTest.cpp)
corrade_add_test(ContainersReferenceStlTest ReferenceStlTest.cpp)
corrade_add_test(ContainersSequenceHelpersTest SequenceHelpersTest.cpp)
corrade_add_test(ContainersScopeGuardTest ScopeGuardTest.cpp)
corrade_add_test(ContainersStaticArrayTest StaticArrayTest.cpp)
corrade_add_test(ContainersStaticArrayViewTest StaticArrayViewTest.cpp)
corrade_add_test(ContainersStaticArrayViewStlTest StaticArrayViewStlTest.cpp)
corrade_add_test(ContainersStridedArrayViewTest StridedArrayViewTest.cpp)
corrade_add_test(ContainersStridedArrayViewStlTest StridedArrayViewStlTest.cpp)
corrade_add_test(ContainersStridedBitArrayViewTest StridedBitArrayViewTest.cpp)
corrade_add_test(ContainersStridedDimensionsTest StridedDimensionsTest.cpp)
corrade_add_test(ContainersStringTest StringTest.cpp LIBRARIES CorradeTestSuiteTestLib)
corrade_add_test(ContainersStringIterableTest StringIterableTest.cpp LIBRARIES CorradeTestSuiteTestLib)
corrade_add_test(ContainersStringStlTest StringStlTest.cpp)
corrade_add_test(ContainersStringViewTest StringViewTest.cpp LIBRARIES CorradeTestSuiteTestLib)

corrade_add_test(ContainersStringViewBenchmark StringViewBenchmark.cpp
    LIBRARIES CorradeTestSuiteTestLib
    FILES StringTestFiles/lorem-ipsum.txt)
target_include_directories(ContainersStringViewBenchmark PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

corrade_add_test(ContainersTripleTest TripleTest.cpp)
corrade_add_test(ContainersTripleStlTest TripleStlTest.cpp)

set_property(TARGET
    ContainersAnyReferenceTest
    ContainersIterableTest
    ContainersLinkedListTest
    ContainersArrayTest
    ContainersArrayViewTest
    ContainersArrayViewStlTest
    ContainersBigEnumSetTest
    ContainersBitArrayTest
    ContainersBitArrayViewTest
    ContainersGrowableArrayTest
    ContainersOptionalTest
    ContainersPointerTest
    ContainersStaticArrayViewTest
    ContainersStridedArrayViewTest
    ContainersStridedBitArrayViewTest
    ContainersStridedDimensionsTest
    ContainersStringTest
    ContainersStringViewTest
    APPEND PROPERTY COMPILE_DEFINITIONS "CORRADE_GRACEFUL_ASSERT")

# Build these only if there's no explicit -std= passed in the flags
if(NOT CMAKE_CXX_FLAGS MATCHES "-std=")
    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.4") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0.1") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
        corrade_add_test(ContainersPairCpp14Test PairCpp14Test.cpp)
        corrade_add_test(ContainersTripleCpp14Test TripleCpp14Test.cpp)
        set_target_properties(
            ContainersPairCpp14Test
            ContainersTripleCpp14Test
            PROPERTIES CORRADE_CXX_STANDARD 14)
   endif()

    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.3") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.10"))
        corrade_add_test(ContainersOptionalStlTest OptionalStlTest.cpp)
        corrade_add_test(ContainersPairCpp17Test PairCpp17Test.cpp)
        corrade_add_test(ContainersStaticArrayCpp17Test StaticArrayCpp17Test.cpp)
        corrade_add_test(ContainersStaticArrayViewCpp17Test StaticArrayViewCpp17Test.cpp)
        corrade_add_test(ContainersStridedDimensionsCpp17Test StridedDimensionsCpp17Test.cpp)
        corrade_add_test(ContainersStringStlViewTest StringStlViewTest.cpp)
        corrade_add_test(ContainersTripleCpp17Test TripleCpp17Test.cpp)
        set_target_properties(
            ContainersOptionalStlTest
            ContainersPairCpp17Test
            ContainersStaticArrayViewCpp17Test
            ContainersStaticArrayCpp17Test
            ContainersStridedDimensionsCpp17Test
            ContainersStringStlViewTest
            ContainersTripleCpp17Test
            PROPERTIES CORRADE_CXX_STANDARD 17)
    endif()

    # Copied verbatim from src/Corrade/Test/CMakeLists.txt, please keep in sync
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0") OR
       (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.15"))
        corrade_add_test(ContainersArrayStlSpanTest ArrayStlSpanTest.cpp)
        corrade_add_test(ContainersArrayViewStlSpanTest ArrayViewStlSpanTest.cpp)
        corrade_add_test(ContainersStaticArrayStlSpanTest StaticArrayStlSpanTest.cpp)
        corrade_add_test(ContainersStaticArrayViewStlSpanTest StaticArrayViewStlSpanTest.cpp)
        set_target_properties(
            ContainersArrayStlSpanTest
            ContainersArrayViewStlSpanTest
            ContainersStaticArrayStlSpanTest
            ContainersStaticArrayViewStlSpanTest
            PROPERTIES CORRADE_CXX_STANDARD 20)
    endif()
endif()
