/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <set>      /* for unique allocation counting in a benchmark */
#include <vector>   /* for benchmark against STL */

#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"
#include "Corrade/TestSuite/Compare/Container.h"
#include "Corrade/TestSuite/Compare/Numeric.h"
#include "Corrade/TestSuite/Compare/String.h"
#include "Corrade/Utility/Format.h"

/* No __has_feature on GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60512
   Using a dedicated macro instead: https://stackoverflow.com/a/34814667 */
#ifdef __has_feature
#if __has_feature(address_sanitizer)
#define _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif
#endif
#ifdef __SANITIZE_ADDRESS__
#define _CORRADE_CONTAINERS_SANITIZER_ENABLED
#endif

/* For malloc() / realloc() failure tests, as TSan doesn't like those either */
#ifdef __has_feature
#if __has_feature(thread_sanitizer)
#define _CORRADE_CONTAINERS_THREAD_SANITIZER_ENABLED
#endif
#endif
#ifdef __SANITIZE_THREAD__
#define _CORRADE_CONTAINERS_THREAD_SANITIZER_ENABLED
#endif

#ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
/* https://github.com/llvm-mirror/compiler-rt/blob/master/include/sanitizer/common_interface_defs.h */
extern "C" int __sanitizer_verify_contiguous_container(const void *beg,
    const void *mid, const void *end);
extern "C" const void *__sanitizer_contiguous_container_find_bad_address(
    const void *beg, const void *mid, const void *end);
#define VERIFY_SANITIZED_PROPERLY(array, Allocator) \
    do {                                                                    \
        bool sanitized = __sanitizer_verify_contiguous_container(Allocator::base(array.begin()), array.end(), array.begin() + Allocator::capacity(array)); \
        if(!sanitized) {                                                    \
            Debug{} << "Sanitization annotation for array of capacity" << Allocator::capacity(array) << "and size" << array.size() << "failed at offset" << reinterpret_cast<const typename Allocator::Type*>(__sanitizer_contiguous_container_find_bad_address(Allocator::base(array.begin()), array.end(), array.begin() + Allocator::capacity(array))) - array.begin(); \
        }                                                                   \
        CORRADE_VERIFY(sanitized);                                          \
    } while(false)
#else
#define VERIFY_SANITIZED_PROPERLY(array, Allocator) do {} while(false)
#endif

namespace Corrade { namespace Containers { namespace Test { namespace {

struct GrowableArrayTest: TestSuite::Tester {
    explicit GrowableArrayTest();

    void resetCounters();

    template<class T> void reserveFromEmpty();
    template<class T> void reserveFromNonGrowable();
    template<class T> void reserveFromNonGrowableNoOp();
    template<class T> void reserveFromGrowable();
    template<class T> void reserveFromGrowableNoOp();

    template<class T> void resizeFromEmpty();
    template<class T> void resizeFromNonGrowable();
    template<class T> void resizeFromNonGrowableNoOp();
    template<class T> void resizeFromGrowable();
    template<class T> void resizeFromGrowableNoOp();
    template<class T> void resizeFromGrowableNoRealloc();

    template<class T> void resizeNoInit();
    #ifdef CORRADE_BUILD_DEPRECATED
    template<class T> void resizeDefaultInit();
    #endif
    template<class T> void resizeValueInit();
    void resizeDirectInit();
    void resizeCopy();

    template<class T, class Init> void resizeFromNonGrowableToLess();
    template<class T, class Init> void resizeFromGrowableToLess();

    template<class T> void appendFromEmpty();
    template<class T> void insertFromEmpty();
    template<class T> void appendFromNonGrowable();
    template<class T> void insertFromNonGrowable();
    template<class T> void appendFromGrowable();
    template<class T> void insertFromGrowable();
    template<class T> void appendFromGrowableNoRealloc();
    template<class T> void insertFromGrowableNoRealloc();
    template<class T> void insertFromGrowableNoReallocNoInit();

    void appendCopy();
    void insertCopy();
    void appendInPlace();
    void insertInPlace();
    void appendMove();
    void insertMove();
    void appendList();
    void insertList();
    void appendListEmpty();
    void insertListEmpty();
    template<class T> void appendCountValueInit();
    void appendCountNoInit();
    void appendCountDirectInit();
    template<class T> void insertCountValueInit();
    void insertCountNoInit();
    void insertCountDirectInit();
    template<class T> void appendCountValueInitEmpty();
    void appendCountNoInitEmpty();
    void appendCountDirectInitEmpty();
    template<class T> void insertCountValueInitEmpty();
    void insertCountNoInitEmpty();
    void insertCountDirectInitEmpty();

    void insertShiftOperationOrder();
    void insertShiftOperationOrderNoOp();
    void insertShiftOperationOrderNoOverlap();
    void insertInvalid();

    void appendGrowRatio();
    /* Insert uses the same growth ratio underneath, no need to test again */

    template<class T> void removeZero();
    template<class T> void removeUnorderedZero();
    template<class T> void removeSuffixZero();
    template<class T> void removeNonGrowable();
    template<class T> void removeUnorderedNonGrowable();
    template<class T> void removeSuffixNonGrowable();
    template<class T> void removeAllNonGrowable();
    template<class T> void removeUnorderedAllNonGrowable();
    template<class T> void removeSuffixAllNonGrowable();
    template<class T> void removeGrowable();
    template<class T> void removeUnorderedGrowable();
    template<class T> void removeSuffixGrowable();
    template<class T> void removeAllGrowable();
    template<class T> void removeUnorderedAllGrowable();
    template<class T> void removeSuffixAllGrowable();

    void removeShiftOperationOrder();
    void removeShiftOperationOrderNoOp();
    void removeShiftOperationOrderNoOverlap();
    void removeUnorderedShiftOperationOrder();
    void removeUnorderedShiftOperationOrderNoOp();
    void removeUnorderedShiftOperationOrderNoOverlap();
    void removeInvalid();

    template<class T> void clearNonGrowable();
    template<class T> void clearGrowable();

    void mallocFailed();
    void reallocFailed();

    template<class T> void shrinkNonGrowableEmptyNoInit();
    #ifdef CORRADE_BUILD_DEPRECATED
    template<class T> void shrinkNonGrowableEmptyDefaultInit();
    #endif
    template<class T> void shrinkNonGrowableEmptyValueInit();
    template<class T> void shrinkNonGrowableNoInit();
    #ifdef CORRADE_BUILD_DEPRECATED
    template<class T> void shrinkNonGrowableDefaultInit();
    #endif
    template<class T> void shrinkNonGrowableValueInit();
    template<class T> void shrinkGrowableEmptyNoInit();
    #ifdef CORRADE_BUILD_DEPRECATED
    template<class T> void shrinkGrowableEmptyDefaultInit();
    #endif
    template<class T> void shrinkGrowableEmptyValueInit();
    template<class T> void shrinkGrowableNoInit();
    #ifdef CORRADE_BUILD_DEPRECATED
    template<class T> void shrinkGrowableDefaultInit();
    #endif
    template<class T> void shrinkGrowableValueInit();

    template<class T> void move();

    void cast();
    void castEmpty();
    void castNonTrivial();
    void castNonGrowable();
    void castInvalid();

    void explicitAllocatorParameter();

    /* Here go the "weird unexpected corner cases" tests */

    void constructorExplicitInCopyInitialization();
    void copyConstructPlainStruct();
    void moveConstructPlainStruct();

    template<template<class> class Allocator, std::size_t alignment> void allocationAlignment();

    void appendInsertConflictingType();
    void appendInsertArrayElement();
    template<bool growable = true> void appendInsertArraySlice();
    void insertArraySliceIntoItself();

    /* Here go the benchmarks */

    void benchmarkAppendVector();
    void benchmarkAppendArray();
    void benchmarkAppendReservedVector();
    void benchmarkAppendReservedArray();

    void benchmarkAppendTrivialVector();
    template<template<class> class Allocator> void benchmarkAppendTrivialArray();
    void benchmarkAppendTrivialReservedVector();
    void benchmarkAppendTrivialReservedArray();

    void benchmarkAppendBatchTrivialVector();
    template<template<class> class Allocator> void benchmarkAppendBatchTrivialArray();
    void benchmarkAppendBatchTrivialReservedVector();
    void benchmarkAppendBatchTrivialReservedArray();

    void benchmarkAllocationsBegin();
    std::uint64_t benchmarkAllocationsEnd();

    void benchmarkAllocationsVector();
    template<template<class> class Allocator> void benchmarkAllocationsArray();
};

const struct {
    const char* name;
    bool growable;
    std::size_t capacityEnd;
} AppendInsertArrayElementData[]{
    {"", true, 19},
    {"non-growable", false, 9}
};

const struct {
    const char* name;
    std::size_t insert;
    std::size_t begin;
    std::size_t expectInvalidBegin, expectInvalidEnd;
    int expected[14];
} AppendInsertArraySliceData[]{
    {"array begin, append",
        ~std::size_t{}, 0, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 40, 50, 60, 70, 80, 90, 00, 10, 20, 30}},
    {"array begin, insert in the middle",
        6, 0, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 40, 50, 00, 10, 20, 30, 60, 70, 80, 90}},
    {"array begin, insert right before the slice",
        0, 0, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 00, 10, 20, 30, 40, 50, 60, 70, 80, 90}},
    {"array begin, insert right after the slice",
        4, 0, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 00, 10, 20, 30, 40, 50, 60, 70, 80, 90}},
    {"array middle, append",
        ~std::size_t{}, 3, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 40, 50, 60, 70, 80, 90, 30, 40, 50, 60}},
    {"array middle, insert before the slice",
        2, 3, ~std::size_t{}, ~std::size_t{},
        {00, 10, 30, 40, 50, 60, 20, 30, 40, 50, 60, 70, 80, 90}},
    {"array middle, insert right before the slice",
        3, 3, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 40, 50, 60, 30, 40, 50, 60, 70, 80, 90}},
    {"array middle, insert after the slice",
        9, 3, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 40, 50, 60, 70, 80, 30, 40, 50, 60, 90}},
    {"array middle, insert right after the slice",
        7, 3, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 40, 50, 60, 30, 40, 50, 60, 70, 80, 90}},
    {"array end, append",
        ~std::size_t{}, 6, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 40, 50, 60, 70, 80, 90, 60, 70, 80, 90}},
    {"array end, insert in the middle",
        3, 6, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 60, 70, 80, 90, 30, 40, 50, 60, 70, 80, 90}},
    {"array end, insert right before the slice",
        6, 6, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 40, 50, 60, 70, 80, 90, 60, 70, 80, 90}},
    {"array end, insert right after the slice",
        10, 6, ~std::size_t{}, ~std::size_t{},
        {00, 10, 20, 30, 40, 50, 60, 70, 80, 90, 60, 70, 80, 90}},
    {"array capacity, append",
        ~std::size_t{}, 8, 12, 14,
        /* There are values 100 and 110 right after the end but because of the
           grow it'll pick up uninitialized memory at position where 120 was
           before                                        vvv  vvv */
        {00, 10, 20, 30, 40, 50, 60, 70, 80, 90, 80, 90, 100, 110}},
    {"array capacity, insert in the middle",
        3, 8, 5, 7,
        /* In this case, depending on how growAtBy() works inside, the last two
           inserted values will be either the original values before the shift
           (so, 50, 60) or uninitialized memory
                             vvv  vvv */
        {00, 10, 20, 80, 90, 100, 110, 30, 40, 50, 60, 70, 80, 90}},
};

struct Movable {
    static int constructed;
    static int destructed;
    static int moved;
    static int assigned;

    /*implicit*/ Movable(int a = 0) noexcept: a{short(a)} { ++constructed; }
    Movable(const Movable&) = delete;
    Movable(Movable&& other) noexcept: a(other.a) {
        ++constructed;
        ++moved;
    }
    ~Movable() {
        /* Catch double frees */
        CORRADE_INTERNAL_ASSERT(!thisDestructed);
        ++destructed;
        thisDestructed = true;
    }
    Movable& operator=(const Movable&) = delete;
    Movable& operator=(Movable&& other) noexcept {
        a = other.a;
        ++assigned;
        ++moved;
        return *this;
    }

    /* "compatibility" with ints */
    Movable& operator=(int value) {
        a = value;
        return *this;
    }
    explicit operator int() const { return a; }

    short a;
    bool thisDestructed = false;
};

static_assert(sizeof(Movable) == 4, "tests require Movable to be four bytes");

int Movable::constructed = 0;
int Movable::destructed = 0;
int Movable::moved = 0;
int Movable::assigned = 0;

static_assert(
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    !__has_trivial_constructor(Movable)
    #else
    !std::is_trivially_constructible<Movable>::value
    #endif
    , "Movable should be testing the non-trivial code path");
static_assert(
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    !__has_trivial_copy(Movable) || !__has_trivial_destructor(Movable)
    #else
    !std::is_trivially_copyable<Movable>::value
    #endif
    , "Movable should be testing the non-trivial code path");

GrowableArrayTest::GrowableArrayTest() {
    addTests({&GrowableArrayTest::reserveFromEmpty<int>,
              &GrowableArrayTest::reserveFromEmpty<Movable>,
              &GrowableArrayTest::reserveFromNonGrowable<int>,
              &GrowableArrayTest::reserveFromNonGrowable<Movable>,
              &GrowableArrayTest::reserveFromNonGrowableNoOp<int>,
              &GrowableArrayTest::reserveFromNonGrowableNoOp<Movable>,
              &GrowableArrayTest::reserveFromGrowable<int>,
              &GrowableArrayTest::reserveFromGrowable<Movable>,
              &GrowableArrayTest::reserveFromGrowableNoOp<int>,
              &GrowableArrayTest::reserveFromGrowableNoOp<Movable>,

              &GrowableArrayTest::resizeFromEmpty<int>,
              &GrowableArrayTest::resizeFromEmpty<Movable>,
              &GrowableArrayTest::resizeFromNonGrowable<int>,
              &GrowableArrayTest::resizeFromNonGrowable<Movable>,
              &GrowableArrayTest::resizeFromNonGrowableNoOp<int>,
              &GrowableArrayTest::resizeFromNonGrowableNoOp<Movable>,
              &GrowableArrayTest::resizeFromGrowable<int>,
              &GrowableArrayTest::resizeFromGrowable<Movable>,
              &GrowableArrayTest::resizeFromGrowableNoOp<int>,
              &GrowableArrayTest::resizeFromGrowableNoOp<Movable>,
              &GrowableArrayTest::resizeFromGrowableNoRealloc<int>,
              &GrowableArrayTest::resizeFromGrowableNoRealloc<Movable>,

              &GrowableArrayTest::resizeNoInit<int>,
              &GrowableArrayTest::resizeNoInit<Movable>,
              #ifdef CORRADE_BUILD_DEPRECATED
              &GrowableArrayTest::resizeDefaultInit<int>,
              &GrowableArrayTest::resizeDefaultInit<Movable>,
              #endif
              &GrowableArrayTest::resizeValueInit<int>,
              &GrowableArrayTest::resizeValueInit<Movable>,
              &GrowableArrayTest::resizeDirectInit,
              &GrowableArrayTest::resizeCopy,

              &GrowableArrayTest::resizeFromNonGrowableToLess<int, Corrade::NoInitT>,
              &GrowableArrayTest::resizeFromNonGrowableToLess<Movable, Corrade::NoInitT>,
              #ifdef CORRADE_BUILD_DEPRECATED
              &GrowableArrayTest::resizeFromNonGrowableToLess<int, Corrade::DefaultInitT>,
              &GrowableArrayTest::resizeFromNonGrowableToLess<Movable, Corrade::DefaultInitT>,
              #endif
              &GrowableArrayTest::resizeFromNonGrowableToLess<int, Corrade::ValueInitT>,
              &GrowableArrayTest::resizeFromNonGrowableToLess<Movable, Corrade::ValueInitT>,
              &GrowableArrayTest::resizeFromNonGrowableToLess<int, Corrade::DirectInitT>,
              &GrowableArrayTest::resizeFromNonGrowableToLess<Movable, Corrade::DirectInitT>,
              &GrowableArrayTest::resizeFromGrowableToLess<int, Corrade::NoInitT>,
              &GrowableArrayTest::resizeFromGrowableToLess<Movable, Corrade::NoInitT>,
              #ifdef CORRADE_BUILD_DEPRECATED
              &GrowableArrayTest::resizeFromGrowableToLess<int, Corrade::DefaultInitT>,
              &GrowableArrayTest::resizeFromGrowableToLess<Movable, Corrade::DefaultInitT>,
              #endif
              &GrowableArrayTest::resizeFromGrowableToLess<int, Corrade::ValueInitT>,
              &GrowableArrayTest::resizeFromGrowableToLess<Movable, Corrade::ValueInitT>,
              &GrowableArrayTest::resizeFromGrowableToLess<int, Corrade::NoInitT>,
              &GrowableArrayTest::resizeFromGrowableToLess<Movable, Corrade::NoInitT>,

              &GrowableArrayTest::appendFromEmpty<int>,
              &GrowableArrayTest::appendFromEmpty<Movable>,
              &GrowableArrayTest::insertFromEmpty<int>,
              &GrowableArrayTest::insertFromEmpty<Movable>,
              &GrowableArrayTest::appendFromNonGrowable<int>,
              &GrowableArrayTest::appendFromNonGrowable<Movable>,
              &GrowableArrayTest::insertFromNonGrowable<int>,
              &GrowableArrayTest::insertFromNonGrowable<Movable>,
              &GrowableArrayTest::appendFromGrowable<int>,
              &GrowableArrayTest::appendFromGrowable<Movable>,
              &GrowableArrayTest::insertFromGrowable<int>,
              &GrowableArrayTest::insertFromGrowable<Movable>,
              &GrowableArrayTest::appendFromGrowableNoRealloc<int>,
              &GrowableArrayTest::appendFromGrowableNoRealloc<Movable>,
              &GrowableArrayTest::insertFromGrowableNoRealloc<int>,
              &GrowableArrayTest::insertFromGrowableNoRealloc<Movable>,
              &GrowableArrayTest::insertFromGrowableNoReallocNoInit<int>,
              &GrowableArrayTest::insertFromGrowableNoReallocNoInit<Movable>,

              &GrowableArrayTest::appendCopy,
              &GrowableArrayTest::insertCopy,
              &GrowableArrayTest::appendInPlace,
              &GrowableArrayTest::insertInPlace,
              &GrowableArrayTest::appendMove,
              &GrowableArrayTest::insertMove,
              &GrowableArrayTest::appendList,
              &GrowableArrayTest::insertList,
              &GrowableArrayTest::appendListEmpty,
              &GrowableArrayTest::insertListEmpty,
              &GrowableArrayTest::appendCountValueInit<int>,
              &GrowableArrayTest::appendCountValueInit<Movable>,
              &GrowableArrayTest::appendCountNoInit,
              &GrowableArrayTest::appendCountDirectInit,
              &GrowableArrayTest::insertCountValueInit<int>,
              &GrowableArrayTest::insertCountValueInit<Movable>,
              &GrowableArrayTest::insertCountNoInit,
              &GrowableArrayTest::insertCountDirectInit,
              &GrowableArrayTest::appendCountValueInitEmpty<int>,
              &GrowableArrayTest::appendCountValueInitEmpty<Movable>,
              &GrowableArrayTest::appendCountNoInitEmpty,
              &GrowableArrayTest::appendCountDirectInitEmpty,
              &GrowableArrayTest::insertCountValueInitEmpty<int>,
              &GrowableArrayTest::insertCountValueInitEmpty<Movable>,
              &GrowableArrayTest::insertCountNoInitEmpty,
              &GrowableArrayTest::insertCountDirectInitEmpty,

              &GrowableArrayTest::insertShiftOperationOrder,
              &GrowableArrayTest::insertShiftOperationOrderNoOp,
              &GrowableArrayTest::insertShiftOperationOrderNoOverlap,
              &GrowableArrayTest::insertInvalid,

              &GrowableArrayTest::appendGrowRatio,

              &GrowableArrayTest::removeZero<int>,
              &GrowableArrayTest::removeZero<Movable>,
              &GrowableArrayTest::removeUnorderedZero<int>,
              &GrowableArrayTest::removeUnorderedZero<Movable>,
              &GrowableArrayTest::removeSuffixZero<int>,
              &GrowableArrayTest::removeSuffixZero<Movable>,
              &GrowableArrayTest::removeNonGrowable<int>,
              &GrowableArrayTest::removeNonGrowable<Movable>,
              &GrowableArrayTest::removeUnorderedNonGrowable<int>,
              &GrowableArrayTest::removeUnorderedNonGrowable<Movable>,
              &GrowableArrayTest::removeSuffixNonGrowable<int>,
              &GrowableArrayTest::removeSuffixNonGrowable<Movable>,
              &GrowableArrayTest::removeAllNonGrowable<int>,
              &GrowableArrayTest::removeAllNonGrowable<Movable>,
              &GrowableArrayTest::removeUnorderedAllNonGrowable<int>,
              &GrowableArrayTest::removeUnorderedAllNonGrowable<Movable>,
              &GrowableArrayTest::removeSuffixAllNonGrowable<int>,
              &GrowableArrayTest::removeSuffixAllNonGrowable<Movable>,
              &GrowableArrayTest::removeGrowable<int>,
              &GrowableArrayTest::removeGrowable<Movable>,
              &GrowableArrayTest::removeUnorderedGrowable<int>,
              &GrowableArrayTest::removeUnorderedGrowable<Movable>,
              &GrowableArrayTest::removeSuffixGrowable<int>,
              &GrowableArrayTest::removeSuffixGrowable<Movable>,
              &GrowableArrayTest::removeAllGrowable<int>,
              &GrowableArrayTest::removeAllGrowable<Movable>,
              &GrowableArrayTest::removeUnorderedAllGrowable<int>,
              &GrowableArrayTest::removeUnorderedAllGrowable<Movable>,
              &GrowableArrayTest::removeSuffixAllGrowable<int>,
              &GrowableArrayTest::removeSuffixAllGrowable<Movable>,

              &GrowableArrayTest::removeShiftOperationOrder,
              &GrowableArrayTest::removeShiftOperationOrderNoOp,
              &GrowableArrayTest::removeShiftOperationOrderNoOverlap,
              &GrowableArrayTest::removeUnorderedShiftOperationOrder,
              &GrowableArrayTest::removeUnorderedShiftOperationOrderNoOp,
              &GrowableArrayTest::removeUnorderedShiftOperationOrderNoOverlap,
              &GrowableArrayTest::removeInvalid,

              &GrowableArrayTest::clearNonGrowable<int>,
              &GrowableArrayTest::clearNonGrowable<Movable>,
              &GrowableArrayTest::clearGrowable<int>,
              &GrowableArrayTest::clearGrowable<Movable>,

              &GrowableArrayTest::mallocFailed,
              &GrowableArrayTest::reallocFailed,

              &GrowableArrayTest::shrinkNonGrowableEmptyNoInit<int>,
              &GrowableArrayTest::shrinkNonGrowableEmptyNoInit<Movable>,
              #ifdef CORRADE_BUILD_DEPRECATED
              &GrowableArrayTest::shrinkNonGrowableEmptyDefaultInit<int>,
              &GrowableArrayTest::shrinkNonGrowableEmptyDefaultInit<Movable>,
              #endif
              &GrowableArrayTest::shrinkNonGrowableEmptyValueInit<int>,
              &GrowableArrayTest::shrinkNonGrowableEmptyValueInit<Movable>,
              &GrowableArrayTest::shrinkNonGrowableNoInit<int>,
              &GrowableArrayTest::shrinkNonGrowableNoInit<Movable>,
              #ifdef CORRADE_BUILD_DEPRECATED
              &GrowableArrayTest::shrinkNonGrowableDefaultInit<int>,
              &GrowableArrayTest::shrinkNonGrowableDefaultInit<Movable>,
              #endif
              &GrowableArrayTest::shrinkNonGrowableValueInit<int>,
              &GrowableArrayTest::shrinkNonGrowableValueInit<Movable>,
              &GrowableArrayTest::shrinkGrowableEmptyNoInit<int>,
              &GrowableArrayTest::shrinkGrowableEmptyNoInit<Movable>,
              #ifdef CORRADE_BUILD_DEPRECATED
              &GrowableArrayTest::shrinkGrowableEmptyDefaultInit<int>,
              &GrowableArrayTest::shrinkGrowableEmptyDefaultInit<Movable>,
              #endif
              &GrowableArrayTest::shrinkGrowableEmptyValueInit<int>,
              &GrowableArrayTest::shrinkGrowableEmptyValueInit<Movable>,
              &GrowableArrayTest::shrinkGrowableNoInit<int>,
              &GrowableArrayTest::shrinkGrowableNoInit<Movable>,
              #ifdef CORRADE_BUILD_DEPRECATED
              &GrowableArrayTest::shrinkGrowableDefaultInit<int>,
              &GrowableArrayTest::shrinkGrowableDefaultInit<Movable>,
              #endif
              &GrowableArrayTest::shrinkGrowableValueInit<int>,
              &GrowableArrayTest::shrinkGrowableValueInit<Movable>,

              &GrowableArrayTest::move<int>,
              &GrowableArrayTest::move<Movable>},
        &GrowableArrayTest::resetCounters, &GrowableArrayTest::resetCounters);

    addTests({&GrowableArrayTest::cast,
              &GrowableArrayTest::castEmpty,
              &GrowableArrayTest::castNonTrivial,
              &GrowableArrayTest::castNonGrowable,
              &GrowableArrayTest::castInvalid,

              &GrowableArrayTest::explicitAllocatorParameter,

              &GrowableArrayTest::constructorExplicitInCopyInitialization,
              &GrowableArrayTest::copyConstructPlainStruct,
              &GrowableArrayTest::moveConstructPlainStruct});

    addRepeatedTests<GrowableArrayTest>({
        &GrowableArrayTest::allocationAlignment<ArrayNewAllocator, 1>,
        &GrowableArrayTest::allocationAlignment<ArrayNewAllocator, 2>,
        &GrowableArrayTest::allocationAlignment<ArrayNewAllocator, 4>,
        &GrowableArrayTest::allocationAlignment<ArrayNewAllocator, 8>,
        &GrowableArrayTest::allocationAlignment<ArrayNewAllocator, 16>,
        &GrowableArrayTest::allocationAlignment<ArrayMallocAllocator, 1>,
        &GrowableArrayTest::allocationAlignment<ArrayMallocAllocator, 2>,
        &GrowableArrayTest::allocationAlignment<ArrayMallocAllocator, 4>,
        &GrowableArrayTest::allocationAlignment<ArrayMallocAllocator, 8>,
        &GrowableArrayTest::allocationAlignment<ArrayMallocAllocator, 16>}, 100);

    addTests({&GrowableArrayTest::appendInsertConflictingType});

    addInstancedTests({&GrowableArrayTest::appendInsertArrayElement},
        Containers::arraySize(AppendInsertArrayElementData));

    addInstancedTests<GrowableArrayTest>({
        &GrowableArrayTest::appendInsertArraySlice,
        &GrowableArrayTest::appendInsertArraySlice<false>},
        Containers::arraySize(AppendInsertArraySliceData));

    addTests({&GrowableArrayTest::insertArraySliceIntoItself});

    addBenchmarks({
        &GrowableArrayTest::benchmarkAppendVector,
        &GrowableArrayTest::benchmarkAppendArray,
        &GrowableArrayTest::benchmarkAppendReservedVector,
        &GrowableArrayTest::benchmarkAppendReservedArray,
        &GrowableArrayTest::benchmarkAppendTrivialVector,
        &GrowableArrayTest::benchmarkAppendTrivialArray<ArrayNewAllocator>,
        &GrowableArrayTest::benchmarkAppendTrivialArray<ArrayMallocAllocator>,
        &GrowableArrayTest::benchmarkAppendTrivialReservedVector,
        &GrowableArrayTest::benchmarkAppendTrivialReservedArray,
        &GrowableArrayTest::benchmarkAppendBatchTrivialVector,
        &GrowableArrayTest::benchmarkAppendBatchTrivialArray<ArrayNewAllocator>,
        &GrowableArrayTest::benchmarkAppendBatchTrivialArray<ArrayMallocAllocator>,
        &GrowableArrayTest::benchmarkAppendBatchTrivialReservedVector,
        &GrowableArrayTest::benchmarkAppendBatchTrivialReservedArray}, 10);

    addCustomInstancedBenchmarks({
        &GrowableArrayTest::benchmarkAllocationsVector,
        &GrowableArrayTest::benchmarkAllocationsArray<ArrayNewAllocator>,
        &GrowableArrayTest::benchmarkAllocationsArray<ArrayMallocAllocator>
    }, 1, 3,
        &GrowableArrayTest::benchmarkAllocationsBegin,
        &GrowableArrayTest::benchmarkAllocationsEnd, BenchmarkUnits::Count);

    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    Debug{} << "Address Sanitizer detected, checking container annotations";
    #endif
}

void GrowableArrayTest::resetCounters() {
    Movable::constructed = Movable::destructed = Movable::moved = Movable::assigned = 0;
}

template<class> struct TypeName;
template<> struct TypeName<int> {
    static const char* name() { return "ArrayMallocAllocator"; }
};
template<> struct TypeName<Movable> {
    static const char* name() { return "ArrayNewAllocator"; }
};

template<class T> void GrowableArrayTest::reserveFromEmpty() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        CORRADE_VERIFY(!a); /* pointer is null */
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 0);
        /* Not growable, no ASan annotation check */

        CORRADE_COMPARE(arrayReserve(a, 100), 100);
        CORRADE_VERIFY(a); /* size is 0, but pointer is non-null */
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 100);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* No construction / destruction done */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 0);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 0);
    }
}

template<class T> void GrowableArrayTest::reserveFromNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a;
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        CORRADE_VERIFY(!arrayIsGrowable(a));
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 3);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        /* Not growable, no ASan annotation check */

        CORRADE_COMPARE(arrayReserve(a, 100), 100);
        CORRADE_VERIFY(a != prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 100);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        CORRADE_COMPARE(int(a[2]), 3);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* 3 times constructed initially, then 3 times moved, then all destroyed */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3 + 3);
        CORRADE_COMPARE(Movable::moved, 3);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3 + 3);
    }
}

template<class T> void GrowableArrayTest::reserveFromNonGrowableNoOp() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a;
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 3);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        /* Not growable, no ASan annotation check */

        CORRADE_COMPARE(arrayReserve(a, 3), 3);
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 3);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        CORRADE_COMPARE(int(a[2]), 3);
        /* Not growable, no ASan annotation check */
    }

    /* The reserve was a no-op, so no change */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

template<class T> void GrowableArrayTest::reserveFromGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 3);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        /* Not growable, no ASan annotation check */

        CORRADE_COMPARE(arrayReserve(a, 50), 50);
        T* prev = a;
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 50);
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 3 + 3);
            CORRADE_COMPARE(Movable::moved, 3);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 3);
        }
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        CORRADE_COMPARE(arrayReserve(a, 100), 100);
        CORRADE_VERIFY(arrayIsGrowable(a));
        /* std::realloc() for ints might extend it in-place */
        if(std::is_same<T, Movable>::value)
            CORRADE_VERIFY(a != prev);
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 100);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        CORRADE_COMPARE(int(a[2]), 3);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* Growing an existing array twice, so 3x construction & destruction */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3 + 3 + 3);
        CORRADE_COMPARE(Movable::moved, 3 + 3);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3 + 3 + 3);
    }
}

template<class T> void GrowableArrayTest::reserveFromGrowableNoOp() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 3);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        /* Not growable, no ASan annotation check */

        CORRADE_COMPARE(arrayReserve(a, 100), 100);
        T* prev = a;
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 100);
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 3 + 3);
            CORRADE_COMPARE(Movable::moved, 3);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 3);
        }
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        CORRADE_COMPARE(arrayReserve(a, 99), 100);
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 100);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        CORRADE_COMPARE(int(a[2]), 3);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The second reserve should do nothing */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3 + 3);
        CORRADE_COMPARE(Movable::moved, 3);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3 + 3);
    }
}

template<class T> void GrowableArrayTest::resizeFromEmpty() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayResize(a, 3);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 3);
        CORRADE_COMPARE(int(a[0]), 0);
        CORRADE_COMPARE(int(a[1]), 0);
        CORRADE_COMPARE(int(a[2]), 0);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* Only construction (and destruction) should take place, no moves */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

template<class T> void GrowableArrayTest::resizeFromNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{2};
        a[0] = 1;
        a[1] = 2;
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 2);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        /* Not growable, no ASan annotation check */

        arrayResize(a, 4);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 4);
        CORRADE_COMPARE(arrayCapacity(a), 4);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        CORRADE_COMPARE(int(a[2]), 0);
        CORRADE_COMPARE(int(a[3]), 0);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* Old items move-constructed and the new ones constructed in-place */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 2 + 4);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 2 + 4);
    }
}

template<class T> void GrowableArrayTest::resizeFromNonGrowableNoOp() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a;
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 3);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        /* Not growable, no ASan annotation check */

        arrayResize(a, 3);
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 3);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        CORRADE_COMPARE(int(a[2]), 3);
        /* Not growable, no ASan annotation check */
    }

    /* No change was done to the array */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

template<class T> void GrowableArrayTest::resizeFromGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    /* Should behave the same as resizeFromNonGrowable() */

    {
        Array<T> a;
        arrayResize(a, 2);
        T* prev = a;
        a[0] = 1;
        a[1] = 2;
        CORRADE_VERIFY(arrayIsGrowable(a));
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 2);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayResize(a, 4);
        /* std::realloc() for ints might extend it in-place */
        if(std::is_same<T, Movable>::value)
            CORRADE_VERIFY(a != prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 4);
        CORRADE_COMPARE(arrayCapacity(a), 4);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        CORRADE_COMPARE(int(a[2]), 0);
        CORRADE_COMPARE(int(a[3]), 0);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* Old items move-constructed and the new one constructed in place */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 2 + 4);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 2 + 4);
    }
}

template<class T> void GrowableArrayTest::resizeFromGrowableNoOp() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayResize(a, 3);
        T* prev = a;
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        CORRADE_VERIFY(arrayIsGrowable(a));
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 3);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayResize(a, 3);
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 3);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        CORRADE_COMPARE(int(a[2]), 3);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* No change was done to the array */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

template<class T> void GrowableArrayTest::resizeFromGrowableNoRealloc() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 4);
        CORRADE_VERIFY(arrayIsGrowable(a));
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
        T* prev = a;
        arrayResize(a, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        a[0] = 1;
        a[1] = 2;
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        CORRADE_COMPARE(arrayCapacity(a), 4);
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 2);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayResize(a, 4);
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 4);
        CORRADE_COMPARE(arrayCapacity(a), 4);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        CORRADE_COMPARE(int(a[2]), 0);
        CORRADE_COMPARE(int(a[3]), 0);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The second resize should do nothing except changing size */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 4);
    }
}

template<class T> void GrowableArrayTest::resizeNoInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    Array<int> a;
    arrayResize(a, Corrade::NoInit, 3);
    CORRADE_COMPARE(a.size(), 3);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);

    /* Welp. The contents can be kinda anything, so */
}

#ifdef CORRADE_BUILD_DEPRECATED
template<class T> void GrowableArrayTest::resizeDefaultInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    Array<T> a;
    CORRADE_IGNORE_DEPRECATED_PUSH
    arrayResize(a, Corrade::DefaultInit, 3);
    CORRADE_IGNORE_DEPRECATED_POP
    CORRADE_COMPARE(a.size(), 3);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

    /* Welp. The contents can be kinda anything for ints, so test just Movable */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(int(a[0]), 0);
        CORRADE_COMPARE(int(a[1]), 0);
        CORRADE_COMPARE(int(a[2]), 0);
    }
}
#endif

template<class T> void GrowableArrayTest::resizeValueInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    Array<T> a;
    arrayResize(a, Corrade::ValueInit, 3);
    CORRADE_COMPARE(a.size(), 3);
    CORRADE_COMPARE(int(a[0]), 0);
    CORRADE_COMPARE(int(a[1]), 0);
    CORRADE_COMPARE(int(a[2]), 0);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
}

void GrowableArrayTest::resizeDirectInit() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice */

    Array<Movable> a;
    /* Passing a Movable rvalue instead of just an int to verify it gets
       correctly forwarded */
    arrayResize(a, Corrade::DirectInit, 3, Movable{-31601});
    CORRADE_COMPARE(a.size(), 3);
    CORRADE_COMPARE(int(a[0]), -31601);
    CORRADE_COMPARE(int(a[1]), -31601);
    CORRADE_COMPARE(int(a[2]), -31601);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<Movable>);
}

void GrowableArrayTest::resizeCopy() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice */

    Array<int> a;
    arrayResize(a, 3, 754831);
    CORRADE_COMPARE(a.size(), 3);
    CORRADE_COMPARE(a[0], 754831);
    CORRADE_COMPARE(a[1], 754831);
    CORRADE_COMPARE(a[2], 754831);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
}

template<class> struct InitTagName;
template<> struct InitTagName<Corrade::NoInitT> {
    static const char* name() { return "NoInit"; }
};
#ifdef CORRADE_BUILD_DEPRECATED
template<> struct InitTagName<Corrade::DefaultInitT> {
    static const char* name() { return "DefaultInitT"; }
};
#endif
template<> struct InitTagName<Corrade::ValueInitT> {
    static const char* name() { return "ValueInitT"; }
};
template<> struct InitTagName<Corrade::DirectInitT> {
    static const char* name() { return "DirectInitT"; }
};

template<class T, class Init> void GrowableArrayTest::resizeFromNonGrowableToLess() {
    setTestCaseTemplateName({TypeName<T>::name(), InitTagName<Init>::name()});

    {
        Array<T> a{4};
        T* prev = a;
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        a[3] = 4;
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 4);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }
        /* Not growable, no ASan annotation check */

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @todo for the deprecated DefaultInit variant, remove when gone */
        CORRADE_IGNORE_DEPRECATED_PUSH
        #endif
        arrayResize(a, Init{typename Init::Init{}}, 2);
        #ifdef CORRADE_BUILD_DEPRECATED
        CORRADE_IGNORE_DEPRECATED_POP
        #endif
        CORRADE_VERIFY(a != prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        CORRADE_COMPARE(arrayCapacity(a), 2);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The resize move-constructed just the remaining elements */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 4 + 2);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 4 + 2);
    }
}

template<class T, class Init> void GrowableArrayTest::resizeFromGrowableToLess() {
    setTestCaseTemplateName({TypeName<T>::name(), InitTagName<Init>::name()});

    {
        Array<T> a;
        arrayResize(a, 4);
        T* prev = a;
        a[0] = 1;
        a[1] = 2;
        a[2] = 3;
        a[3] = 4;
        CORRADE_VERIFY(arrayIsGrowable(a));
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 4);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }

        #ifdef CORRADE_BUILD_DEPRECATED
        /** @todo for the deprecated DefaultInit variant, remove when gone */
        CORRADE_IGNORE_DEPRECATED_PUSH
        #endif
        arrayResize(a, Init{typename Init::Init{}}, 2);
        #ifdef CORRADE_BUILD_DEPRECATED
        CORRADE_IGNORE_DEPRECATED_POP
        #endif
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        CORRADE_COMPARE(arrayCapacity(a), 4);
        CORRADE_COMPARE(int(a[0]), 1);
        CORRADE_COMPARE(int(a[1]), 2);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The resize only called half of the destructors early */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 4);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 2);
        }
    }

    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 4);
    }
}

template<class T> void GrowableArrayTest::appendFromEmpty() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        T& appended = arrayAppend(a, T{37});
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 1);
        #ifndef CORRADE_TARGET_32BIT
        CORRADE_COMPARE(arrayCapacity(a), 2);
        /** @todo expose Implementation::DefaultAllocationAlignment instead */
        #elif !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
        CORRADE_COMPARE(arrayCapacity(a), 1);
        #else
        CORRADE_COMPARE(arrayCapacity(a), 3);
        #endif
        CORRADE_COMPARE(int(a[0]), 37);
        CORRADE_COMPARE(&appended, &a.back());
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The 37 is constructed as a temporary and then move-constructed into the
       new place */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::moved, 1);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 2);
    }
}

template<class T> void GrowableArrayTest::insertFromEmpty() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        T& inserted = arrayInsert(a, 0, T{37});
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 1);
        #ifndef CORRADE_TARGET_32BIT
        CORRADE_COMPARE(arrayCapacity(a), 2);
        /** @todo expose Implementation::DefaultAllocationAlignment instead */
        #elif !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
        CORRADE_COMPARE(arrayCapacity(a), 1);
        #else
        CORRADE_COMPARE(arrayCapacity(a), 3);
        #endif
        CORRADE_COMPARE(int(a[0]), 37);
        CORRADE_COMPARE(&inserted, &a[0]);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The 37 is constructed as a temporary and then move-constructed into the
       new place */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::moved, 1);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 2);
    }
}

template<class T> void GrowableArrayTest::appendFromNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{1};
        T* prev = a;
        a[0] = 28;
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 1);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }

        T& appended = arrayAppend(a, T{37});
        CORRADE_VERIFY(a != prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        #ifndef CORRADE_TARGET_32BIT
        CORRADE_COMPARE(arrayCapacity(a), 2);
        /** @todo expose Implementation::DefaultAllocationAlignment instead */
        #elif !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
        CORRADE_COMPARE(arrayCapacity(a), 2);
        #else
        CORRADE_COMPARE(arrayCapacity(a), 3);
        #endif
        CORRADE_COMPARE(int(a[0]), 28);
        CORRADE_COMPARE(int(a[1]), 37);
        CORRADE_COMPARE(&appended, &a.back());
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The first item is default-constructed in-place. Then, 37 is constructed
       as a temporary. Then, as append reallocates, 28 is move-constructed
       (third construction, first move) to new memory. Then 37 is
       move-constructed (fourth construction, second move) into the new
       place. */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 4);
    }
}

template<class T> void GrowableArrayTest::insertFromNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a;
        a[0] = 28;
        a[1] = 42;
        a[2] = 56;
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 3);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 0);
        }

        T& inserted = arrayInsert(a, 1, T{37});
        CORRADE_VERIFY(a != prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 4);
        CORRADE_COMPARE(arrayCapacity(a), 4);
        CORRADE_COMPARE(int(a[0]), 28);
        CORRADE_COMPARE(int(a[1]), 37);
        CORRADE_COMPARE(int(a[2]), 42);
        CORRADE_COMPARE(int(a[3]), 56);
        CORRADE_COMPARE(&inserted, &a[1]);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The first three items are default-constructed in-place. Then, 37 is
       constructed as a temporary. Then as insert reallocates, 28, 42 and 56 is
       move-constructed into new memory (fifth to seventh construction, first
       to third move), leaving a gap in between. Then 37 is move-constructed
       (eighth construction, fourth move) into the new place. */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 8);
        CORRADE_COMPARE(Movable::moved, 4);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 8);
    }
}

template<class T> void GrowableArrayTest::appendFromGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayResize(a, 1);
        T* prev = a;
        CORRADE_VERIFY(arrayIsGrowable(a));
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        a[0] = 28;
        T& appended = arrayAppend(a, T{37});
        /* std::realloc() for ints might extend it in-place */
        if(std::is_same<T, Movable>::value)
            CORRADE_VERIFY(a != prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        #ifndef CORRADE_TARGET_32BIT
        CORRADE_COMPARE(arrayCapacity(a), 2);
        #else
        CORRADE_COMPARE(arrayCapacity(a), 3);
        #endif
        CORRADE_COMPARE(int(a[0]), 28);
        CORRADE_COMPARE(int(a[1]), 37);
        CORRADE_COMPARE(&appended, &a.back());
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The first item is default-constructed in-place. Then, 37 is constructed
       as a temporary. Then as append reallocates, 28 is move-constructed into
       new memory (third construction, first move). Then 37 is move-constructed
       into the new place (fourth construction, second move). */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 4);
    }
}

template<class T> void GrowableArrayTest::insertFromGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayResize(a, 3);
        T* prev = a;
        CORRADE_VERIFY(arrayIsGrowable(a));
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        a[0] = 28;
        a[1] = 42;
        a[2] = 56;
        T& inserted = arrayInsert(a, 1, T{37});
        /* std::realloc() for ints might extend it in-place */
        if(std::is_same<T, Movable>::value)
            CORRADE_VERIFY(a != prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 4);
        #ifndef CORRADE_TARGET_32BIT
        CORRADE_COMPARE(arrayCapacity(a), 8);
        #else
        CORRADE_COMPARE(arrayCapacity(a), 7);
        #endif
        CORRADE_COMPARE(int(a[0]), 28);
        CORRADE_COMPARE(int(a[1]), 37);
        CORRADE_COMPARE(int(a[2]), 42);
        CORRADE_COMPARE(int(a[3]), 56);
        CORRADE_COMPARE(&inserted, &a[1]);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The first three items are default-constructed in-place. Then, 37 is
       constructed as a temporary. Then, as insert reallocates, 28, 42 and 56
       is move-constructed into new memory (fifth to seventh construction,
       first to third move), 56 move-constructed (eighth construction, fourth
       move) to a new place, 42 move-assigned (fifth move, first assignment) to
       where 56 was before, and the moved-out gap after 42 is destructed. Then,
       37 is move-constructed into the new place (9th construction, 6th move). */
    /** @todo fix to do the "widening" move directly during reallocation and
        not in a subsequent step; change destruction + move-construction to
        a move-assignment */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 9);
        CORRADE_COMPARE(Movable::moved, 6);
        CORRADE_COMPARE(Movable::assigned, 1);
        CORRADE_COMPARE(Movable::destructed, 9);
    }
}

template<class T> void GrowableArrayTest::appendFromGrowableNoRealloc() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 2);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
        T* prev = a;
        arrayResize(a, 1);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
        a[0] = 28;
        T& appended = arrayAppend(a, T{37});
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        CORRADE_COMPARE(arrayCapacity(a), 2);
        CORRADE_COMPARE(int(a[0]), 28);
        CORRADE_COMPARE(int(a[1]), 37);
        CORRADE_COMPARE(&appended, &a.back());
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The first item is constructed in-place. Then, 37 is constructed as a
       temporary and then move-constructed into the new place. */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 1);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

template<class T> void GrowableArrayTest::insertFromGrowableNoRealloc() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 4);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
        T* prev = a;
        arrayResize(a, 3);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
        a[0] = 28;
        a[1] = 42;
        a[2] = 56;
        T& inserted = arrayInsert(a, 1, T{37});
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 4);
        CORRADE_COMPARE(arrayCapacity(a), 4);
        CORRADE_COMPARE(int(a[0]), 28);
        CORRADE_COMPARE(int(a[1]), 37);
        CORRADE_COMPARE(int(a[2]), 42);
        CORRADE_COMPARE(int(a[3]), 56);
        CORRADE_COMPARE(&inserted, &a[1]);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The first three items are constructed in-place. Then, 37 is constructed
       as a temporary. Then, 56 is move-constructed (fifth construction, first
       move) to a new place, 42 is move-assigned (second move, first
       assignment) to where 56 was before, and the moved-out gap after 42 is
       destructed. Then, 37 is move-constructed to where 42 was before (6th construction, third move). */
    /** @todo fix destruction + move-construction to a move-assignment */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::moved, 3);
        CORRADE_COMPARE(Movable::assigned, 1);
        CORRADE_COMPARE(Movable::destructed, 6);
    }
}

template<class T> void GrowableArrayTest::insertFromGrowableNoReallocNoInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 4);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
        T* prev = a;
        arrayResize(a, 3);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
        a[0] = 28;
        a[1] = 42;
        a[2] = 56;
        T& inserted = arrayInsert(a, 1, Corrade::NoInit, 1).front();
        new(&inserted) T{37};
        CORRADE_VERIFY(a == prev);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 4);
        CORRADE_COMPARE(arrayCapacity(a), 4);
        CORRADE_COMPARE(int(a[0]), 28);
        CORRADE_COMPARE(int(a[1]), 37);
        CORRADE_COMPARE(int(a[2]), 42);
        CORRADE_COMPARE(int(a[3]), 56);
        CORRADE_COMPARE(&inserted, &a[1]);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);
    }

    /* The first three items are constructed in-place. Then, 56 is
       move-constructed (fourth construction, first move) to a new place, 42 is
       move-assigned (second move, first assignment) to where 56 was before,
       and the moved-out gap after 42 is destructed. Then, 37 is constructed
       in-place (fifth construction).

       Compared to insertFromGrowableNoRealloc(), the destruction of the gap
       after 42 has to happen so users can always perform a placement-new
       without having to worry about what was there before. */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 5);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 1);
        CORRADE_COMPARE(Movable::destructed, 5);
    }
}

void GrowableArrayTest::appendCopy() {
    Array<int> a;
    int& appended = arrayAppend(a, 2786541);
    CORRADE_COMPARE(a.size(), 1);
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(arrayCapacity(a), 2);
    /** @todo expose Implementation::DefaultAllocationAlignment instead */
    #elif !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_COMPARE(arrayCapacity(a), 1);
    #else
    CORRADE_COMPARE(arrayCapacity(a), 3);
    #endif
    CORRADE_COMPARE(a[0], 2786541);
    CORRADE_COMPARE(&appended, &a.back());
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
}

void GrowableArrayTest::insertCopy() {
    Array<int> a;
    int& inserted = arrayInsert(a, 0, 2786541);
    CORRADE_COMPARE(a.size(), 1);
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(arrayCapacity(a), 2);
    /** @todo expose Implementation::DefaultAllocationAlignment instead */
    #elif !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_COMPARE(arrayCapacity(a), 1);
    #else
    CORRADE_COMPARE(arrayCapacity(a), 3);
    #endif
    CORRADE_COMPARE(a[0], 2786541);
    CORRADE_COMPARE(&inserted, &a.back());
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
}

void GrowableArrayTest::appendInPlace() {
    {
        Array<Movable> a;
        /* Passing a Movable and not just int to test that the rvalue gets
           correctly forwarded and move constructor called */
        Movable& appended = arrayAppend(a, Corrade::InPlaceInit, Movable{25141});
        CORRADE_COMPARE(a.size(), 1);
        #ifndef CORRADE_TARGET_32BIT
        CORRADE_COMPARE(arrayCapacity(a), 2);
        /** @todo expose Implementation::DefaultAllocationAlignment instead */
        #elif !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
        CORRADE_COMPARE(arrayCapacity(a), 1);
        #else
        CORRADE_COMPARE(arrayCapacity(a), 3);
        #endif
        CORRADE_COMPARE(a[0].a, 25141);
        CORRADE_COMPARE(&appended, &a.back());
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<Movable>);
    }

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 2);
}

void GrowableArrayTest::insertInPlace() {
    {
        Array<Movable> a;
        /* Passing a Movable and not just int to test that the rvalue gets
           correctly forwarded and move constructor called */
        Movable& inserted = arrayInsert(a, 0, Corrade::InPlaceInit, Movable{25141});
        CORRADE_COMPARE(a.size(), 1);
        #ifndef CORRADE_TARGET_32BIT
        CORRADE_COMPARE(arrayCapacity(a), 2);
        /** @todo expose Implementation::DefaultAllocationAlignment instead */
        #elif !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
        CORRADE_COMPARE(arrayCapacity(a), 1);
        #else
        CORRADE_COMPARE(arrayCapacity(a), 3);
        #endif
        CORRADE_COMPARE(a[0].a, 25141);
        CORRADE_COMPARE(&inserted, &a.back());
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<Movable>);
    }

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 2);
}

void GrowableArrayTest::appendMove() {
    {
        Array<Movable> a;
        Movable& appended = arrayAppend(a, Movable{25141});
        CORRADE_COMPARE(a.size(), 1);
        #ifndef CORRADE_TARGET_32BIT
        CORRADE_COMPARE(arrayCapacity(a), 2);
        /** @todo expose Implementation::DefaultAllocationAlignment instead */
        #elif !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
        CORRADE_COMPARE(arrayCapacity(a), 1);
        #else
        CORRADE_COMPARE(arrayCapacity(a), 3);
        #endif
        CORRADE_COMPARE(a[0].a, 25141);
        CORRADE_COMPARE(&appended, &a.back());
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<Movable>);
    }

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 2);
}

void GrowableArrayTest::insertMove() {
    {
        Array<Movable> a;
        Movable& inserted = arrayInsert(a, 0, Movable{25141});
        CORRADE_COMPARE(a.size(), 1);
        #ifndef CORRADE_TARGET_32BIT
        CORRADE_COMPARE(arrayCapacity(a), 2);
        /** @todo expose Implementation::DefaultAllocationAlignment instead */
        #elif !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
        CORRADE_COMPARE(arrayCapacity(a), 1);
        #else
        CORRADE_COMPARE(arrayCapacity(a), 3);
        #endif
        CORRADE_COMPARE(a[0].a, 25141);
        CORRADE_COMPARE(&inserted, &a.back());
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<Movable>);
    }

    CORRADE_COMPARE(Movable::constructed, 2);
    CORRADE_COMPARE(Movable::moved, 1);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 2);
}

void GrowableArrayTest::appendList() {
    /* This isn't templated on an allocator because copy-initializing from
       Movable wouldn't be possible */

    Array<int> a;
    Containers::ArrayView<int> appended = arrayAppend(a, {17, -22, 65, 2786541});
    CORRADE_COMPARE(a.size(), 4);
    CORRADE_COMPARE(arrayCapacity(a), 4); /** @todo use growing here too */
    CORRADE_COMPARE(a[0], 17);
    CORRADE_COMPARE(a[1], -22);
    CORRADE_COMPARE(a[2], 65);
    CORRADE_COMPARE(a[3], 2786541);
    CORRADE_COMPARE(appended.data(), a.data());
    CORRADE_COMPARE(appended.size(), 4);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
}

void GrowableArrayTest::insertList() {
    /* This isn't templated on an allocator because copy-initializing from
       Movable wouldn't be possible */

    Array<int> a;
    Containers::ArrayView<int> inserted = arrayInsert(a, 0, {17, -22, 65, 2786541});
    CORRADE_COMPARE(a.size(), 4);
    CORRADE_COMPARE(arrayCapacity(a), 4); /** @todo use growing here too */
    CORRADE_COMPARE(a[0], 17);
    CORRADE_COMPARE(a[1], -22);
    CORRADE_COMPARE(a[2], 65);
    CORRADE_COMPARE(a[3], 2786541);
    CORRADE_COMPARE(inserted.data(), a.data());
    CORRADE_COMPARE(inserted.size(), 4);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
}

void GrowableArrayTest::appendListEmpty() {
    Array<int> a{3};
    int* prev = a.data();
    Containers::ArrayView<int> appended = arrayAppend(a, {});

    /* Should be a no-op, not reallocating the (non-growable) array */
    CORRADE_COMPARE(a.size(), 3);
    CORRADE_COMPARE(a.data(), prev);
    CORRADE_COMPARE(appended.data(), a.end());
    CORRADE_COMPARE(appended.size(), 0);
}

void GrowableArrayTest::insertListEmpty() {
    Array<int> a{3};
    int* prev = a.data();
    Containers::ArrayView<int> inserted = arrayInsert(a, 1, {});

    /* Should be a no-op, not reallocating the (non-growable) array */
    CORRADE_COMPARE(a.size(), 3);
    CORRADE_COMPARE(a.data(), prev);
    CORRADE_COMPARE(inserted.data(), &a[1]);
    CORRADE_COMPARE(inserted.size(), 0);
}

template<class T> void GrowableArrayTest::appendCountValueInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 2);
        arrayAppend(a, T{17});
        arrayAppend(a, T{-22});
        Containers::ArrayView<T> appended = arrayAppend(a, Corrade::ValueInit, 4);
        CORRADE_COMPARE(a.size(), 6);
        CORRADE_COMPARE(arrayCapacity(a), 6); /** @todo use growing here too */
        CORRADE_COMPARE(int(a[0]), 17);
        CORRADE_COMPARE(int(a[1]), -22);
        CORRADE_COMPARE(int(a[2]), 0);
        CORRADE_COMPARE(int(a[3]), 0);
        CORRADE_COMPARE(int(a[4]), 0);
        CORRADE_COMPARE(int(a[5]), 0);
        CORRADE_COMPARE(appended.data(), a.data() + 2);
        CORRADE_COMPARE(appended.size(), 4);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* Construction, move-construction and then (move-)reallocation of the
           two items should be happening, and then construction of the four
           appended */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 6 + 4);
            CORRADE_COMPARE(Movable::moved, 4);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 4);
        }
    }

    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 6 + 4);
        CORRADE_COMPARE(Movable::moved, 4);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 4 + 6);
    }
}

void GrowableArrayTest::appendCountNoInit() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice. However want to verify that the elements indeed
       don't get constructed, so using a Movable. */

    {
        Array<Movable> a;
        arrayReserve(a, 2);
        arrayAppend(a, Movable{17});
        arrayAppend(a, Movable{-22});
        Containers::ArrayView<Movable> appended = arrayAppend(a, Corrade::NoInit, 4);
        CORRADE_COMPARE(a.size(), 6);
        CORRADE_COMPARE(arrayCapacity(a), 6); /** @todo use growing here too */
        CORRADE_COMPARE(int(a[0]), 17);
        CORRADE_COMPARE(int(a[1]), -22);
        CORRADE_COMPARE(appended.data(), a.data() + 2);
        CORRADE_COMPARE(appended.size(), 4);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<Movable>);

        /* Construction, move-construction and then (move-)reallocation of the
           two items should be happening, nothing else */
        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::moved, 4);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 4);

        /* Call the constructors so the destructor doesn't randomly blow up
           thinking double destruction was happening */
        for(Movable& i: appended)
            new(&i) Movable{};
    }

    CORRADE_COMPARE(Movable::constructed, 6 + 4);
    CORRADE_COMPARE(Movable::moved, 4);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 4 + 2 + 4);
}

void GrowableArrayTest::appendCountDirectInit() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice. However want to verify that the elements indeed get
       constructed, so using a Movable. */

    {
        Array<Movable> a;
        arrayReserve(a, 2);
        arrayAppend(a, Movable{17});
        arrayAppend(a, Movable{-22});
        /* Passing a Movable rvalue instead of just an int to verify it gets
           correctly forwarded */
        Containers::ArrayView<Movable> appended = arrayAppend(a, Corrade::DirectInit, 4, Movable{-1337});
        CORRADE_COMPARE(a.size(), 6);
        CORRADE_COMPARE(arrayCapacity(a), 6); /** @todo use growing here too */
        CORRADE_COMPARE(int(a[0]), 17);
        CORRADE_COMPARE(int(a[1]), -22);
        CORRADE_COMPARE(int(a[2]), -1337);
        CORRADE_COMPARE(int(a[3]), -1337);
        CORRADE_COMPARE(int(a[4]), -1337);
        CORRADE_COMPARE(int(a[5]), -1337);
        CORRADE_COMPARE(appended.data(), a.data() + 2);
        CORRADE_COMPARE(appended.size(), 4);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<Movable>);

        /* Construction, move-construction and then (move-)reallocation of the
           two items should be happening, and then construction of the four
           appended, moved four times from the argument */
        CORRADE_COMPARE(Movable::constructed, 6 + 5);
        CORRADE_COMPARE(Movable::moved, 4 + 4);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 5);
    }

    CORRADE_COMPARE(Movable::constructed, 6 + 5);
    CORRADE_COMPARE(Movable::moved, 4 + 4);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 5 + 6);
}

template<class T> void GrowableArrayTest::insertCountValueInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 2);
        arrayAppend(a, T{17});
        arrayAppend(a, T{-22});
        Containers::ArrayView<T> inserted = arrayInsert(a, 1, Corrade::ValueInit, 4);
        CORRADE_COMPARE(a.size(), 6);
        CORRADE_COMPARE(arrayCapacity(a), 6); /** @todo use growing here too */
        CORRADE_COMPARE(int(a[0]), 17);
        CORRADE_COMPARE(int(a[1]), 0);
        CORRADE_COMPARE(int(a[2]), 0);
        CORRADE_COMPARE(int(a[3]), 0);
        CORRADE_COMPARE(int(a[4]), 0);
        CORRADE_COMPARE(int(a[5]), -22);
        CORRADE_COMPARE(inserted.data(), a.data() + 1);
        CORRADE_COMPARE(inserted.size(), 4);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* Construction, move-construction, (move-)reallocation of the two
           items and then another move of the last item for insert should be
           happening, and then construction of the four inserted */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 7 + 4);
            CORRADE_COMPARE(Movable::moved, 5);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 5);
        }
    }

    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 7 + 4);
        CORRADE_COMPARE(Movable::moved, 5);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 5 + 6);
    }
}

void GrowableArrayTest::insertCountNoInit() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice. However want to verify that the elements indeed
       don't get constructed, so using a Movable. */

    {
        Array<Movable> a;
        arrayReserve(a, 2);
        arrayAppend(a, Movable{17});
        arrayAppend(a, Movable{-22});
        Containers::ArrayView<Movable> inserted = arrayInsert(a, 1, Corrade::NoInit, 4);
        CORRADE_COMPARE(a.size(), 6);
        CORRADE_COMPARE(arrayCapacity(a), 6); /** @todo use growing here too */
        CORRADE_COMPARE(int(a[0]), 17);
        CORRADE_COMPARE(int(a[5]), -22);
        CORRADE_COMPARE(inserted.data(), a.data() + 1);
        CORRADE_COMPARE(inserted.size(), 4);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<Movable>);

        /* Construction, move-construction, (move-)reallocation of the two
           items and then another move of the last item for insert should be
           happening, nothing else */
        CORRADE_COMPARE(Movable::constructed, 7);
        CORRADE_COMPARE(Movable::moved, 5);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 5);

        /* Call the constructors so the destructor doesn't randomly blow up
           thinking double destruction was happening */
        for(Movable& i: inserted)
            new(&i) Movable{};
    }

    CORRADE_COMPARE(Movable::constructed, 7 + 4);
    CORRADE_COMPARE(Movable::moved, 5);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 5 + 2 + 4);
}

void GrowableArrayTest::insertCountDirectInit() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice. However want to verify that the elements indeed get
       constructed, so using a Movable. */

    {
        Array<Movable> a;
        arrayReserve(a, 2);
        arrayAppend(a, Movable{17});
        arrayAppend(a, Movable{-22});
        /* Passing a Movable rvalue instead of just an int to verify it gets
           correctly forwarded */
        Containers::ArrayView<Movable> inserted = arrayInsert(a, 1, Corrade::DirectInit, 4, Movable{-1337});
        CORRADE_COMPARE(a.size(), 6);
        CORRADE_COMPARE(arrayCapacity(a), 6); /** @todo use growing here too */
        CORRADE_COMPARE(int(a[0]), 17);
        CORRADE_COMPARE(int(a[1]), -1337);
        CORRADE_COMPARE(int(a[2]), -1337);
        CORRADE_COMPARE(int(a[3]), -1337);
        CORRADE_COMPARE(int(a[4]), -1337);
        CORRADE_COMPARE(int(a[5]), -22);
        CORRADE_COMPARE(inserted.data(), a.data() + 1);
        CORRADE_COMPARE(inserted.size(), 4);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<Movable>);

        /* Construction, move-construction, (move-)reallocation of the two
           items and then another move of the last item for insert should be
           happening, and then construction of the four inserted, moved four
           times from the argument */
        CORRADE_COMPARE(Movable::constructed, 7 + 5);
        CORRADE_COMPARE(Movable::moved, 5 + 4);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 6);
    }

    CORRADE_COMPARE(Movable::constructed, 7 + 5);
    CORRADE_COMPARE(Movable::moved, 5 + 4);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 6 + 6);
}

template<class T> void GrowableArrayTest::appendCountValueInitEmpty() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a.data();
        Containers::ArrayView<T> appended = arrayAppend(a, Corrade::ValueInit, 0);

        /* Should be a no-op, not reallocating the (non-growable) array */
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(a.data(), prev);
        CORRADE_COMPARE(appended.data(), a.end());
        CORRADE_COMPARE(appended.size(), 0);
    }

    /* No construction or anything else should be happening apart from the
       initial creation */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

void GrowableArrayTest::appendCountNoInitEmpty() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice. However want to verify that the elements indeed
       don't get constructed, so using a Movable. */

    {
        Array<Movable> a{3};
        Movable* prev = a.data();
        Containers::ArrayView<Movable> appended = arrayAppend(a, Corrade::NoInit, 0);

        /* Should be a no-op, not reallocating the (non-growable) array */
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(a.data(), prev);
        CORRADE_COMPARE(appended.data(), a.end());
        CORRADE_COMPARE(appended.size(), 0);
    }

    /* No construction or anything else should be happening apart from the
       initial creation */
    CORRADE_COMPARE(Movable::constructed, 3);
    CORRADE_COMPARE(Movable::moved, 0);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 3);
}

void GrowableArrayTest::appendCountDirectInitEmpty() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice. However want to verify that the elements indeed
       don't get constructed, so using a Movable. */

    {
        Array<Movable> a{3};
        Movable* prev = a.data();
        Containers::ArrayView<Movable> appended = arrayAppend(a, Corrade::DirectInit, 0, -1337);

        /* Should be a no-op, not reallocating the (non-growable) array */
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(a.data(), prev);
        CORRADE_COMPARE(appended.data(), a.end());
        CORRADE_COMPARE(appended.size(), 0);
    }

    /* No construction or anything else should be happening apart from the
       initial creation */
    CORRADE_COMPARE(Movable::constructed, 3);
    CORRADE_COMPARE(Movable::moved, 0);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 3);
}

template<class T> void GrowableArrayTest::insertCountValueInitEmpty() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a.data();
        Containers::ArrayView<T> inserted = arrayInsert(a, 1, Corrade::ValueInit, 0);

        /* Should be a no-op, not reallocating the (non-growable) array */
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(a.data(), prev);
        CORRADE_COMPARE(inserted.data(), &a[1]);
        CORRADE_COMPARE(inserted.size(), 0);
    }

    /* No construction or anything else should be happening apart from the
       initial creation */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

void GrowableArrayTest::insertCountNoInitEmpty() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice. However want to verify that the elements indeed
       don't get constructed, so using a Movable. */

    {
        Array<Movable> a{3};
        Movable* prev = a.data();
        Containers::ArrayView<Movable> inserted = arrayInsert(a, 1, Corrade::NoInit, 0);

        /* Should be a no-op, not reallocating the (non-growable) array */
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(a.data(), prev);
        CORRADE_COMPARE(inserted.data(), &a[1]);
        CORRADE_COMPARE(inserted.size(), 0);
    }

    /* No construction or anything else should be happening apart from the
       initial creation */
    CORRADE_COMPARE(Movable::constructed, 3);
    CORRADE_COMPARE(Movable::moved, 0);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 3);
}

void GrowableArrayTest::insertCountDirectInitEmpty() {
    /* This doesn't have any special handling for trivial/non-trivial types, no
       need to test twice. However want to verify that the elements indeed
       don't get constructed, so using a Movable. */

    {
        Array<Movable> a{3};
        Movable* prev = a.data();
        Containers::ArrayView<Movable> inserted = arrayInsert(a, 1, Corrade::DirectInit, 0, -1337);

        /* Should be a no-op, not reallocating the (non-growable) array */
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(a.data(), prev);
        CORRADE_COMPARE(inserted.data(), &a[1]);
        CORRADE_COMPARE(inserted.size(), 0);
    }

    /* No construction or anything else should be happening apart from the
       initial creation */
    CORRADE_COMPARE(Movable::constructed, 3);
    CORRADE_COMPARE(Movable::moved, 0);
    CORRADE_COMPARE(Movable::assigned, 0);
    CORRADE_COMPARE(Movable::destructed, 3);
}

struct VerboseMovable {
    /*implicit*/ VerboseMovable(int a = 0) noexcept: a{short(a)} {
        Utility::Debug{} << "Constructing" << a;
    }
    VerboseMovable(const VerboseMovable&) = delete;
    VerboseMovable(VerboseMovable&& other) noexcept: a(other.a) {
        Utility::Debug{} << (other.movedOut ? "Move-constructing (moved-out)" : "Move-constructing") << a << "with a" << (this - &other) << Utility::Debug::nospace << "-element offset";
        other.movedOut = true;
    }
    ~VerboseMovable() {
        Utility::Debug{} << (movedOut ? "Destructing (moved-out)" : "Destructing") << a;
    }
    VerboseMovable& operator=(const VerboseMovable&) = delete;
    VerboseMovable& operator=(VerboseMovable&& other) noexcept {
        a = other.a;
        Utility::Debug{} << (other.movedOut ? "Move-assigning (moved-out)" : "Move-assigning") << a << "with a" << (this - &other) << Utility::Debug::nospace << "-element offset";
        movedOut = other.movedOut;
        other.movedOut = true;
        return *this;
    }

    short a;
    bool movedOut = false;
};

void GrowableArrayTest::insertShiftOperationOrder() {
    Containers::String out;
    Debug redirectOutput{&out};
    {
        Array<VerboseMovable> a;
        arrayReserve(a, 7);
        arrayResize(a, Corrade::NoInit, 5);
        new(&a[0]) VerboseMovable{1};
        new(&a[1]) VerboseMovable{2};
        new(&a[2]) VerboseMovable{3};
        new(&a[3]) VerboseMovable{4};
        new(&a[4]) VerboseMovable{5};

        arrayInsert(a, 1, Corrade::NoInit, 2);
        new(&a[1]) VerboseMovable{6};
        new(&a[2]) VerboseMovable{7};
    }
    CORRADE_COMPARE(out,
        "Constructing 1\n"
        "Constructing 2\n"
        "Constructing 3\n"
        "Constructing 4\n"
        "Constructing 5\n"
        "Move-constructing 5 with a 2-element offset\n"
        "Move-constructing 4 with a 2-element offset\n"
        "Move-assigning 3 with a 2-element offset\n"
        "Move-assigning 2 with a 2-element offset\n"
        "Destructing (moved-out) 3\n"
        "Destructing (moved-out) 2\n"
        "Constructing 6\n"
        "Constructing 7\n"
        "Destructing 1\n"
        "Destructing 6\n"
        "Destructing 7\n"
        "Destructing 2\n"
        "Destructing 3\n"
        "Destructing 4\n"
        "Destructing 5\n");
}

void GrowableArrayTest::insertShiftOperationOrderNoOp() {
    Containers::String out;
    Debug redirectOutput{&out};
    {
        Array<VerboseMovable> a;
        arrayResize(a, Corrade::NoInit, 3);
        new(&a[0]) VerboseMovable{1};
        new(&a[1]) VerboseMovable{2};
        new(&a[2]) VerboseMovable{3};

        /* This does nothing, so no loop should get entered */
        arrayInsert(a, 1, Corrade::NoInit, 0);
    }
    CORRADE_COMPARE(out,
        "Constructing 1\n"
        "Constructing 2\n"
        "Constructing 3\n"
        "Destructing 1\n"
        "Destructing 2\n"
        "Destructing 3\n");
}

void GrowableArrayTest::insertShiftOperationOrderNoOverlap() {
    /* Compared to insertShiftOperationOrder(), this isn't doing any move
       assignments */

    Containers::String out;
    Debug redirectOutput{&out};
    {
        Array<VerboseMovable> a;
        arrayReserve(a, 8);
        arrayResize(a, Corrade::NoInit, 3);
        new(&a[0]) VerboseMovable{1};
        new(&a[1]) VerboseMovable{2};
        new(&a[2]) VerboseMovable{3};

        arrayInsert(a, 1, Corrade::NoInit, 5);
        new(&a[1]) VerboseMovable{4};
        new(&a[2]) VerboseMovable{5};
        new(&a[3]) VerboseMovable{6};
        new(&a[4]) VerboseMovable{7};
        new(&a[5]) VerboseMovable{8};
    }
    CORRADE_COMPARE(out,
        "Constructing 1\n"
        "Constructing 2\n"
        "Constructing 3\n"
        "Move-constructing 3 with a 5-element offset\n"
        "Move-constructing 2 with a 5-element offset\n"
        "Destructing (moved-out) 3\n"
        "Destructing (moved-out) 2\n"
        "Constructing 4\n"
        "Constructing 5\n"
        "Constructing 6\n"
        "Constructing 7\n"
        "Constructing 8\n"
        "Destructing 1\n"
        "Destructing 4\n"
        "Destructing 5\n"
        "Destructing 6\n"
        "Destructing 7\n"
        "Destructing 8\n"
        "Destructing 2\n"
        "Destructing 3\n");
}

void GrowableArrayTest::insertInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Array<int> a{5};

    Containers::String out;
    Error redirectError{&out};
    arrayInsert(a, 6, 7);
    CORRADE_COMPARE(out, "Containers::arrayInsert(): can't insert at index 6 into an array of size 5\n");
}

void GrowableArrayTest::appendGrowRatio() {
    Array<int> a;

    /* On 32-bit, the growing is a bit different due to a different size of
       std::size_t */
    #ifndef CORRADE_TARGET_32BIT
    /* Double the size (minus sizeof(T)) until 64 bytes */
    arrayAppend(a, 1);
    CORRADE_COMPARE(arrayCapacity(a), 2);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    arrayAppend(a, 2);
    CORRADE_COMPARE(arrayCapacity(a), 2);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);

    arrayAppend(a, 3);
    CORRADE_COMPARE(arrayCapacity(a), 6);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    arrayAppend(a, {4, 5, 6});
    CORRADE_COMPARE(arrayCapacity(a), 6);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);

    arrayAppend(a, 7);
    CORRADE_COMPARE(arrayCapacity(a), 14);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    arrayAppend(a, {8, 9, 10, 11, 12, 13, 14});
    CORRADE_COMPARE(arrayCapacity(a), 14); /* 14*4 + 8 == 64 */
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);

    /* Add 50% minus sizeof(T) after */
    arrayAppend(a, 15);
    CORRADE_COMPARE(arrayCapacity(a), 22); /* 64*1.5 = 96 = 22*4 + 8 */
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    arrayAppend(a, {16, 17, 18, 19, 20, 21, 22});
    CORRADE_COMPARE(arrayCapacity(a), 22);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);

    arrayAppend(a, 23);
    CORRADE_COMPARE(arrayCapacity(a), 34); /* 96*1.5 = 144 = 34*4 + 8 */
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    #else
    /* Double the size (minus sizeof(T)) until 64 bytes */
    arrayAppend(a, 1);
    /** @todo expose Implementation::DefaultAllocationAlignment instead */
    #if !defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 8 || defined(CORRADE_TARGET_EMSCRIPTEN)
    CORRADE_COMPARE(arrayCapacity(a), 1);
    #else
    CORRADE_COMPARE(arrayCapacity(a), 3);
    #endif
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    arrayAppend(a, {2, 3});
    CORRADE_COMPARE(arrayCapacity(a), 3);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);

    arrayAppend(a, 4);
    CORRADE_COMPARE(arrayCapacity(a), 7);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    arrayAppend(a, {5, 6, 7});
    CORRADE_COMPARE(arrayCapacity(a), 7);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);

    arrayAppend(a, 8);
    CORRADE_COMPARE(arrayCapacity(a), 15);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    arrayAppend(a, {9, 10, 11, 12, 13, 14, 15});
    CORRADE_COMPARE(arrayCapacity(a), 15); /* 15*4 + 4 == 64 */
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);

    /* Add 50% minus sizeof(T) after */
    arrayAppend(a, 16);
    CORRADE_COMPARE(arrayCapacity(a), 23); /* 64*1.5 = 96 = 23*4 + 4 */
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    arrayAppend(a, {17, 18, 19, 20, 21, 22, 23});
    CORRADE_COMPARE(arrayCapacity(a), 23);
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);

    arrayAppend(a, 24);
    CORRADE_COMPARE(arrayCapacity(a), 35); /* 96*1.5 = 144 = 35*4 + 4 */
    VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<int>);
    #endif
}

template<class T> void GrowableArrayTest::removeZero() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 7;
        a[2] = -1;

        /* Should do no nuthin' */
        arrayRemove(a, 3, 0);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_VERIFY(a.data() == prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        CORRADE_COMPARE(int(a[2]), -1);
        /* Not growable, no ASan annotation check */
    }

    /* The three items are constructed in-place, nothing else hould be done by
       the removal */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

template<class T> void GrowableArrayTest::removeUnorderedZero() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 7;
        a[2] = -1;

        /* Should do no nuthin' */
        arrayRemoveUnordered(a, 3, 0);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_VERIFY(a.data() == prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        CORRADE_COMPARE(int(a[2]), -1);
        /* Not growable, no ASan annotation check */
    }

    /* The three items are constructed in-place, nothing else hould be done by
       the removal */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

template<class T> void GrowableArrayTest::removeSuffixZero() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 7;
        a[2] = -1;

        /* Should do no nuthin' */
        arrayRemoveSuffix(a, 0);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_VERIFY(a.data() == prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        CORRADE_COMPARE(int(a[2]), -1);
        /* Not growable, no ASan annotation check */
    }

    /* The three items are constructed in-place, nothing else should be done by
       the removal */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

template<class T> void GrowableArrayTest::removeNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{4};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 7;
        a[2] = -1;
        a[3] = 5786;

        /* Gets converted to growable as otherwise we can't ensure the
           destructors won't be called on removed elements */
        arrayRemove(a, 1, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        CORRADE_COMPARE(arrayCapacity(a), 2);
        CORRADE_VERIFY(a.data() != prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 5786);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The four items are constructed in-place, then the remaining two of
           them move-constructed to a new array, originals destructed */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 6);
            CORRADE_COMPARE(Movable::moved, 2);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 4);
        }
    }

    /* And finally also the two remaining destructed */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 6);
    }
}

template<class T> void GrowableArrayTest::removeUnorderedNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{4};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 7;
        a[2] = -1;
        a[3] = 5786;

        /* Gets converted to growable as otherwise we can't ensure the
           destructors won't be called on removed elements. The observable
           behavior is the same as with arrayRemove(), as the copy has to be
           done anyway and so there's no reason to differ. */
        arrayRemoveUnordered(a, 1, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        CORRADE_COMPARE(arrayCapacity(a), 2);
        CORRADE_VERIFY(a.data() != prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 5786);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The four items are constructed in-place, then the remaining two of
           them move-constructed to a new array, originals destructed */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 6);
            CORRADE_COMPARE(Movable::moved, 2);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 4);
        }
    }

    /* And finally also the two remaining destructed */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 6);
    }
}

template<class T> void GrowableArrayTest::removeSuffixNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{4};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 7;
        a[2] = -1;
        a[3] = 5786;

        /* Gets converted to growable as otherwise we can't ensure the
           destructors won't be called on removed elements */
        arrayRemoveSuffix(a, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        CORRADE_COMPARE(arrayCapacity(a), 2);
        CORRADE_VERIFY(a.data() != prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The four items are constructed in-place, then the remaining two of
           them move-constructed to a new array, originals destructed */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 6);
            CORRADE_COMPARE(Movable::moved, 2);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 4);
        }
    }

    /* And finally also the two remaining destructed */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 6);
    }
}

template<class T> void GrowableArrayTest::removeAllNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        /* With just two 32-bit elements (in case T is int) and a sufficiently
           smart allocator that has specialized handling for 64-bit allocations
           it could happen that the new, zero-capacity growable allocation
           (which is 64 bits as well, just the capacity value alone basically)
           is placed right before the old allocation, resulting in the
           CORRADE_VERIFY(a.data() != prev) check below to fail:

            new alloc   old alloc
            v           v
            +-----------+-----+-----+
            | capacity  |  0  |  1  |
            +-----------+-----+-----+
                       ^
                       new data pointer == old data pointer

           Which is completely okay and not a bug at all, but it makes the test
           fail, and ignoring this failure could mean that actual bugs get
           silently ignored later. Instead making the old allocation >2x larger
           than the new one in a hope that the new allocation either gets put
           into a different bucket or, if the two end up being in the same size
           bucket, there will be enough space between the two to not have the
           old and new data pointer alias:

            new alloc  old alloc
            v          v
            +-----------+-----------------+-----+-----+-----+-----+-----+
            | capacity  | (unused space)  |  0  |  1  |  2  |  3  |  4  |
            +-----------+-----------------+-----+-----+-----+-----+-----+
                        ^                 ^
                        new data pointer  old data pointer                  */
        Array<T> a{5};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 3;
        a[2] = 4;
        a[3] = 7;
        a[4] = 1;

        /* Gets converted to growable as otherwise we can't ensure the
           destructors won't be called on removed elements */
        arrayRemove(a, 0, 5);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 0);
        CORRADE_VERIFY(a.data() != prev);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The five items are constructed in-place, then a new empty growable
           array is constructed, originals destructed */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 5);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 5);
        }
    }

    /* No change after the array goes out of scope */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 5);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 5);
    }
}

template<class T> void GrowableArrayTest::removeUnorderedAllNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        /* With just two 32-bit elements it could happen that the new,
           zero-capacity growable allocation is placed right before the old
           allocation, resulting in a.data() == prev. See the comment in
           removeAllNonGrowable() for details. */
        Array<T> a{5};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 3;
        a[2] = 4;
        a[3] = 7;
        a[4] = 1;

        /* Gets converted to growable as otherwise we can't ensure the
           destructors won't be called on removed elements. The observable
           behavior is again the same as with arrayRemove(), as the copy has to
           be done anyway and so there's no reason to differ. */
        arrayRemoveUnordered(a, 0, 5);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 0);
        CORRADE_VERIFY(a.data() != prev);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The five items are constructed in-place, then a new empty growable
           array is constructed, originals destructed */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 5);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 5);
        }
    }

    /* No change after the array goes out of scope */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 5);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 5);
    }
}

template<class T> void GrowableArrayTest::removeSuffixAllNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        /* With just two 32-bit elements it could happen that the new,
           zero-capacity growable allocation is placed right before the old
           allocation, resulting in a.data() == prev. See the comment in
           removeAllNonGrowable() for details. */
        Array<T> a{5};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 3;
        a[2] = 4;
        a[3] = 7;
        a[4] = 1;

        /* Gets converted to growable as otherwise we can't ensure the
           destructors won't be called on removed elements */
        arrayRemoveSuffix(a, 5);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 0);
        CORRADE_VERIFY(a.data() != prev);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The five items are constructed in-place, then a new empty growable
           array is constructed, originals destructed */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 5);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 5);
        }
    }

    /* No change after the array goes out of scope */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 5);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 5);
    }
}

template<class T> void GrowableArrayTest::removeGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        arrayAppend(a, Corrade::InPlaceInit, -1);
        arrayAppend(a, Corrade::InPlaceInit, 5786);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayRemove(a, 1, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        CORRADE_COMPARE(arrayCapacity(a), 10);
        CORRADE_VERIFY(a.data() == prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 5786);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The four items are constructed in-place. Then, 5786 is moved over 7
           (first assignment, first move) and -1 and the place after 5786 is
           destructed. */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 4);
            CORRADE_COMPARE(Movable::moved, 1);
            CORRADE_COMPARE(Movable::assigned, 1);
            CORRADE_COMPARE(Movable::destructed, 2);
        }
    }

    /* And finally also the two remaining destructed */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::moved, 1);
        CORRADE_COMPARE(Movable::assigned, 1);
        CORRADE_COMPARE(Movable::destructed, 4);
    }
}

template<class T> void GrowableArrayTest::removeUnorderedGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        arrayAppend(a, Corrade::InPlaceInit, -1);
        arrayAppend(a, Corrade::InPlaceInit, 15);
        arrayAppend(a, Corrade::InPlaceInit, 4);
        arrayAppend(a, Corrade::InPlaceInit, 5786);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayRemoveUnordered(a, 1, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 4);
        CORRADE_COMPARE(arrayCapacity(a), 10);
        CORRADE_VERIFY(a.data() == prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 4);
        CORRADE_COMPARE(int(a[2]), 5786);
        CORRADE_COMPARE(int(a[3]), 15);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The six items are constructed in-place. Then, 4 is moved over 7
           and 5786 over -1 (two assignments, two moves) and the places left
           after them get destructed. */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 6);
            CORRADE_COMPARE(Movable::moved, 2);
            CORRADE_COMPARE(Movable::assigned, 2);
            CORRADE_COMPARE(Movable::destructed, 2);
        }
    }

    /* And finally also the four remaining destructed */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::moved, 2);
        CORRADE_COMPARE(Movable::assigned, 2);
        CORRADE_COMPARE(Movable::destructed, 6);
    }
}

template<class T> void GrowableArrayTest::removeSuffixGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        arrayAppend(a, Corrade::InPlaceInit, -1);
        arrayAppend(a, Corrade::InPlaceInit, 5786);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayRemoveSuffix(a, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 2);
        CORRADE_COMPARE(arrayCapacity(a), 10);
        CORRADE_VERIFY(a.data() == prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The four items are constructed in-place. Then, just two elements of
           those are destructed. */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 4);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 2);
        }
    }

    /* And finally also the two remaining destructed */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 4);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 4);
    }
}

template<class T> void GrowableArrayTest::removeAllGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayRemove(a, 0, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 10);
        CORRADE_VERIFY(a.data() == prev);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The two items are constructed in-place. Then, all are destructed. */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 2);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 2);
        }
    }

    /* No change after the array goes out of scope */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 2);
    }
}

template<class T> void GrowableArrayTest::removeUnorderedAllGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayRemoveUnordered(a, 0, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 10);
        CORRADE_VERIFY(a.data() == prev);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The two items are constructed in-place. Then, all are destructed. */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 2);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 2);
        }
    }

    /* No change after the array goes out of scope */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 2);
    }
}

template<class T> void GrowableArrayTest::removeSuffixAllGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayRemoveSuffix(a, 2);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 10);
        CORRADE_VERIFY(a.data() == prev);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The two items are constructed in-place. Then, all are destructed. */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 2);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 2);
        }
    }

    /* No change after the array goes out of scope */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 2);
    }
}

void GrowableArrayTest::removeShiftOperationOrder() {
    Containers::String out;
    Debug redirectOutput{&out};
    {
        Array<VerboseMovable> a;
        arrayResize(a, Corrade::NoInit, 6);
        new(&a[0]) VerboseMovable{1};
        new(&a[1]) VerboseMovable{2};
        new(&a[2]) VerboseMovable{3};
        new(&a[3]) VerboseMovable{4};
        new(&a[4]) VerboseMovable{5};
        new(&a[5]) VerboseMovable{6};

        arrayRemove(a, 1, 2);
    }
    CORRADE_COMPARE(out,
        "Constructing 1\n"
        "Constructing 2\n"
        "Constructing 3\n"
        "Constructing 4\n"
        "Constructing 5\n"
        "Constructing 6\n"
        "Move-assigning 4 with a -2-element offset\n"
        "Move-assigning 5 with a -2-element offset\n"
        "Move-assigning 6 with a -2-element offset\n"
        "Destructing (moved-out) 5\n"
        "Destructing (moved-out) 6\n"
        "Destructing 1\n"
        "Destructing 4\n"
        "Destructing 5\n"
        "Destructing 6\n");
}

void GrowableArrayTest::removeShiftOperationOrderNoOp() {
    Containers::String out;
    Debug redirectOutput{&out};
    {
        Array<VerboseMovable> a;
        arrayResize(a, Corrade::NoInit, 3);
        new(&a[0]) VerboseMovable{1};
        new(&a[1]) VerboseMovable{2};
        new(&a[2]) VerboseMovable{3};

        /* This does nothing, so no loop should get entered */
        arrayRemove(a, 1, 0);
    }
    CORRADE_COMPARE(out,
        "Constructing 1\n"
        "Constructing 2\n"
        "Constructing 3\n"
        "Destructing 1\n"
        "Destructing 2\n"
        "Destructing 3\n");
}

void GrowableArrayTest::removeShiftOperationOrderNoOverlap() {
    /* Compared to removeShiftOperationOrder(), this has one item that's
       directly destructed without being moved anywhere */

    Containers::String out;
    Debug redirectOutput{&out};
    {
        Array<VerboseMovable> a;
        arrayResize(a, Corrade::NoInit, 6);
        new(&a[0]) VerboseMovable{1};
        new(&a[1]) VerboseMovable{2};
        new(&a[2]) VerboseMovable{3};
        new(&a[3]) VerboseMovable{4};
        new(&a[4]) VerboseMovable{5};
        new(&a[5]) VerboseMovable{6};

        arrayRemove(a, 1, 3);
    }
    CORRADE_COMPARE(out,
        "Constructing 1\n"
        "Constructing 2\n"
        "Constructing 3\n"
        "Constructing 4\n"
        "Constructing 5\n"
        "Constructing 6\n"
        "Move-assigning 5 with a -3-element offset\n"
        "Move-assigning 6 with a -3-element offset\n"
        "Destructing 4\n"
        "Destructing (moved-out) 5\n"
        "Destructing (moved-out) 6\n"
        "Destructing 1\n"
        "Destructing 5\n"
        "Destructing 6\n");
}

void GrowableArrayTest::removeUnorderedShiftOperationOrder() {
    Containers::String out;
    Debug redirectOutput{&out};
    {
        Array<VerboseMovable> a;
        arrayResize(a, Corrade::NoInit, 6);
        new(&a[0]) VerboseMovable{1};
        new(&a[1]) VerboseMovable{2};
        new(&a[2]) VerboseMovable{3};
        new(&a[3]) VerboseMovable{4};
        new(&a[4]) VerboseMovable{5};
        new(&a[5]) VerboseMovable{6};

        arrayRemoveUnordered(a, 1, 2);
    }
    CORRADE_COMPARE(out,
        "Constructing 1\n"
        "Constructing 2\n"
        "Constructing 3\n"
        "Constructing 4\n"
        "Constructing 5\n"
        "Constructing 6\n"
        "Move-assigning 5 with a -3-element offset\n"
        "Move-assigning 6 with a -3-element offset\n"
        "Destructing (moved-out) 5\n"
        "Destructing (moved-out) 6\n"
        "Destructing 1\n"
        "Destructing 5\n"
        "Destructing 6\n"
        "Destructing 4\n");
}

void GrowableArrayTest::removeUnorderedShiftOperationOrderNoOp() {
    Containers::String out;
    Debug redirectOutput{&out};
    {
        Array<VerboseMovable> a;
        arrayResize(a, Corrade::NoInit, 3);
        new(&a[0]) VerboseMovable{1};
        new(&a[1]) VerboseMovable{2};
        new(&a[2]) VerboseMovable{3};

        /* This does nothing, so no loop should get entered */
        arrayRemoveUnordered(a, 1, 0);
    }
    CORRADE_COMPARE(out,
        "Constructing 1\n"
        "Constructing 2\n"
        "Constructing 3\n"
        "Destructing 1\n"
        "Destructing 2\n"
        "Destructing 3\n");
}

void GrowableArrayTest::removeUnorderedShiftOperationOrderNoOverlap() {
    /* Compared to removeUnorderedShiftOperationOrder(), this doesn't have
       `count` items after the removed ones, so one is destructed without being
       moved anywhere. The observed behavior is the same as with
       removeShiftOperationOrderNoOverlap(). */

    Containers::String out;
    Debug redirectOutput{&out};
    {
        Array<VerboseMovable> a;
        arrayResize(a, Corrade::NoInit, 6);
        new(&a[0]) VerboseMovable{1};
        new(&a[1]) VerboseMovable{2};
        new(&a[2]) VerboseMovable{3};
        new(&a[3]) VerboseMovable{4};
        new(&a[4]) VerboseMovable{5};
        new(&a[5]) VerboseMovable{6};

        arrayRemoveUnordered(a, 1, 3);
    }
    CORRADE_COMPARE(out,
        "Constructing 1\n"
        "Constructing 2\n"
        "Constructing 3\n"
        "Constructing 4\n"
        "Constructing 5\n"
        "Constructing 6\n"
        "Move-assigning 5 with a -3-element offset\n"
        "Move-assigning 6 with a -3-element offset\n"
        "Destructing 4\n"
        "Destructing (moved-out) 5\n"
        "Destructing (moved-out) 6\n"
        "Destructing 1\n"
        "Destructing 5\n"
        "Destructing 6\n");
}

void GrowableArrayTest::removeInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Array<int> a{4};

    Containers::String out;
    Error redirectOutput{&out};
    arrayRemove(a, 4, 1);
    arrayRemove(a, 2, 3);
    arrayRemoveUnordered(a, 4, 1);
    arrayRemoveUnordered(a, 2, 3);
    arrayRemoveSuffix(a, 5);
    CORRADE_COMPARE(out,
        "Containers::arrayRemove(): can't remove 1 elements at index 4 from an array of size 4\n"
        "Containers::arrayRemove(): can't remove 3 elements at index 2 from an array of size 4\n"
        "Containers::arrayRemoveUnordered(): can't remove 1 elements at index 4 from an array of size 4\n"
        "Containers::arrayRemoveUnordered(): can't remove 3 elements at index 2 from an array of size 4\n"
        "Containers::arrayRemoveSuffix(): can't remove 5 elements from an array of size 4\n");
}

template<class T> void GrowableArrayTest::clearNonGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{2};
        a[0] = 2;
        a[1] = 3;

        /* Compared to arrayRemove() etc gets just deallocated, with no new
           growable allocation */
        arrayClear(a);
        CORRADE_VERIFY(!a.data());
        CORRADE_VERIFY(!a.deleter());
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 0);

        /* The two items are constructed in-place, then destructed */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 2);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 2);
        }
    }

    /* No change after the array goes out of scope */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 2);
    }
}

template<class T> void GrowableArrayTest::clearGrowable() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        arrayClear(a);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_COMPARE(arrayCapacity(a), 10);
        CORRADE_VERIFY(a.data() == prev);
        VERIFY_SANITIZED_PROPERLY(a, ArrayAllocator<T>);

        /* The two items are constructed in-place. Then, all are destructed. */
        if(std::is_same<T, Movable>::value) {
            CORRADE_COMPARE(Movable::constructed, 2);
            CORRADE_COMPARE(Movable::moved, 0);
            CORRADE_COMPARE(Movable::assigned, 0);
            CORRADE_COMPARE(Movable::destructed, 2);
        }
    }

    /* No change after the array goes out of scope */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 2);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 2);
    }
}

void GrowableArrayTest::mallocFailed() {
    CORRADE_SKIP_IF_NO_ASSERT();
    #if defined(_CORRADE_CONTAINERS_SANITIZER_ENABLED) || defined(_CORRADE_CONTAINERS_THREAD_SANITIZER_ENABLED)
    CORRADE_SKIP("AddressSanitizer or ThreadSanitizer enabled, can't test");
    #endif

    Array<char> a;

    Containers::String out;
    Error redirectOutput{&out};
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 7
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Walloc-size-larger-than="
    #endif
    arrayReserve(a, ~std::size_t{} - Implementation::AllocatorTraits<char>::Offset);
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 7
    #pragma GCC diagnostic pop
    #endif
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out, "Containers::ArrayMallocAllocator: can't allocate 18446744073709551615 bytes\n");
    #else
    CORRADE_COMPARE(out, "Containers::ArrayMallocAllocator: can't allocate 4294967295 bytes\n");
    #endif
}

void GrowableArrayTest::reallocFailed() {
    CORRADE_SKIP_IF_NO_ASSERT();
    #if defined(_CORRADE_CONTAINERS_SANITIZER_ENABLED) || defined(_CORRADE_CONTAINERS_THREAD_SANITIZER_ENABLED)
    CORRADE_SKIP("AddressSanitizer or ThreadSanitizer enabled, can't test");
    #endif

    Array<char> a;
    arrayAppend(a, '3');

    Containers::String out;
    Error redirectOutput{&out};
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 7
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Walloc-size-larger-than="
    #endif
    arrayReserve(a, ~std::size_t{} - Implementation::AllocatorTraits<char>::Offset);
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__ >= 7
    #pragma GCC diagnostic pop
    #endif
    #ifndef CORRADE_TARGET_32BIT
    CORRADE_COMPARE(out, "Containers::ArrayMallocAllocator: can't reallocate 18446744073709551615 bytes\n");
    #else
    CORRADE_COMPARE(out, "Containers::ArrayMallocAllocator: can't reallocate 4294967295 bytes\n");
    #endif
}

template<class T> void GrowableArrayTest::shrinkNonGrowableEmptyNoInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;

        /* Should do no nuthin' */
        arrayShrink(a);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_VERIFY(!a.data());
    }

    /* Nothing should be done by the shrink */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 0);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 0);
    }
}

#ifdef CORRADE_BUILD_DEPRECATED
template<class T> void GrowableArrayTest::shrinkNonGrowableEmptyDefaultInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;

        /* Should do no nuthin' */
        CORRADE_IGNORE_DEPRECATED_PUSH
        arrayShrink(a, Corrade::DefaultInit);
        CORRADE_IGNORE_DEPRECATED_POP
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_VERIFY(!a.data());
    }

    /* Nothing should be done by the shrink */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 0);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 0);
    }
}
#endif

template<class T> void GrowableArrayTest::shrinkNonGrowableEmptyValueInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;

        /* Should do no nuthin' */
        arrayShrink(a, Corrade::ValueInit);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 0);
        CORRADE_VERIFY(!a.data());
    }

    /* Nothing should be done by the shrink */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 0);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 0);
    }
}

template<class T> void GrowableArrayTest::shrinkNonGrowableNoInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 7;
        a[2] = -1;

        /* Should do no nuthin' */
        arrayShrink(a);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_VERIFY(a.data() == prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        CORRADE_COMPARE(int(a[2]), -1);
        /* Not growable, no ASan annotation check */
    }

    /* Nothing should be done by the shrink */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

#ifdef CORRADE_BUILD_DEPRECATED
template<class T> void GrowableArrayTest::shrinkNonGrowableDefaultInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 7;
        a[2] = -1;

        /* Should do no nuthin' */
        CORRADE_IGNORE_DEPRECATED_PUSH
        arrayShrink(a, Corrade::DefaultInit);
        CORRADE_IGNORE_DEPRECATED_POP
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_VERIFY(a.data() == prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        CORRADE_COMPARE(int(a[2]), -1);
        /* Not growable, no ASan annotation check */
    }

    /* Nothing should be done by the shrink */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}
#endif

template<class T> void GrowableArrayTest::shrinkNonGrowableValueInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a{3};
        T* prev = a.data();
        a[0] = 2;
        a[1] = 7;
        a[2] = -1;

        /* Should do no nuthin' */
        arrayShrink(a, Corrade::ValueInit);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_VERIFY(a.data() == prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        CORRADE_COMPARE(int(a[2]), -1);
        /* Not growable, no ASan annotation check */
    }

    /* Nothing should be done by the shrink */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 3);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 3);
    }
}

template<class T> void GrowableArrayTest::shrinkGrowableEmptyNoInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayRemoveSuffix(a, 1);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_VERIFY(a.isEmpty());
        CORRADE_VERIFY(arrayCapacity(a));

        arrayShrink(a);
    }

    /* Nothing extra should be done by the shrink */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 1);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 1);
    }
}

#ifdef CORRADE_BUILD_DEPRECATED
template<class T> void GrowableArrayTest::shrinkGrowableEmptyDefaultInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayRemoveSuffix(a, 1);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_VERIFY(a.isEmpty());
        CORRADE_VERIFY(arrayCapacity(a));

        CORRADE_IGNORE_DEPRECATED_PUSH
        arrayShrink(a, Corrade::DefaultInit);
        CORRADE_IGNORE_DEPRECATED_POP
    }

    /* Nothing extra should be done by the shrink */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 1);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 1);
    }
}
#endif

template<class T> void GrowableArrayTest::shrinkGrowableEmptyValueInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayRemoveSuffix(a, 1);
        CORRADE_VERIFY(arrayIsGrowable(a));
        CORRADE_VERIFY(a.isEmpty());
        CORRADE_VERIFY(arrayCapacity(a));

        arrayShrink(a, Corrade::ValueInit);
    }

    /* Nothing extra should be done by the shrink */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 1);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 1);
    }
}

template<class T> void GrowableArrayTest::shrinkGrowableNoInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        arrayAppend(a, Corrade::InPlaceInit, -1);

        /* Should convert to non-growable */
        arrayShrink(a);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 3);
        CORRADE_VERIFY(a.data() != prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        CORRADE_COMPARE(int(a[2]), -1);
        /* Not growable, no ASan annotation check */
    }

    /* Shrink moves everything to a new array */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::moved, 3);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 6);
    }
}

#ifdef CORRADE_BUILD_DEPRECATED
template<class T> void GrowableArrayTest::shrinkGrowableDefaultInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        arrayAppend(a, Corrade::InPlaceInit, -1);

        /* Should convert to non-growable */
        CORRADE_IGNORE_DEPRECATED_PUSH
        arrayShrink(a, Corrade::DefaultInit);
        CORRADE_IGNORE_DEPRECATED_POP
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 3);
        CORRADE_VERIFY(a.data() != prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        CORRADE_COMPARE(int(a[2]), -1);
        /* Not growable, no ASan annotation check */
    }

    /* Compared to shrinkGrowableDefaultInit(), instead of constructing
       in-place we default-construct and then assign, so three more assignments
       in addition */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::moved, 3);
        CORRADE_COMPARE(Movable::assigned, 3);
        CORRADE_COMPARE(Movable::destructed, 6);
    }
}
#endif

template<class T> void GrowableArrayTest::shrinkGrowableValueInit() {
    setTestCaseTemplateName(TypeName<T>::name());

    {
        Array<T> a;
        arrayReserve(a, 10);
        T* prev = a.data();
        arrayAppend(a, Corrade::InPlaceInit, 2);
        arrayAppend(a, Corrade::InPlaceInit, 7);
        arrayAppend(a, Corrade::InPlaceInit, -1);

        /* Should convert to non-growable */
        arrayShrink(a, Corrade::ValueInit);
        CORRADE_VERIFY(!arrayIsGrowable(a));
        CORRADE_COMPARE(a.size(), 3);
        CORRADE_COMPARE(arrayCapacity(a), 3);
        CORRADE_VERIFY(a.data() != prev);
        CORRADE_COMPARE(int(a[0]), 2);
        CORRADE_COMPARE(int(a[1]), 7);
        CORRADE_COMPARE(int(a[2]), -1);
        /* Not growable, no ASan annotation check */
    }

    /* Compared to shrinkGrowableDefaultInit(), instead of constructing
       in-place we default-construct and then assign, so three more assignments
       in addition */
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 6);
        CORRADE_COMPARE(Movable::moved, 3);
        CORRADE_COMPARE(Movable::assigned, 3);
        CORRADE_COMPARE(Movable::destructed, 6);
    }
}

template<class T> void GrowableArrayTest::move() {
    setTestCaseTemplateName(TypeName<T>::name());

    Array<T> a;
    arrayResize(a, 10);
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 10);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 0);
    }

    Array<T> b = Utility::move(a);
    CORRADE_VERIFY(arrayIsGrowable(b));
    CORRADE_VERIFY(!arrayIsGrowable(a));
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 10);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 0);
    }

    Array<T> c{10};
    c = Utility::move(b);
    CORRADE_VERIFY(arrayIsGrowable(c));
    CORRADE_VERIFY(!arrayIsGrowable(b));
    if(std::is_same<T, Movable>::value) {
        CORRADE_COMPARE(Movable::constructed, 20);
        CORRADE_COMPARE(Movable::moved, 0);
        CORRADE_COMPARE(Movable::assigned, 0);
        CORRADE_COMPARE(Movable::destructed, 0);
    }
}

void GrowableArrayTest::cast() {
    Array<char> a;
    arrayResize(a, 10);

    auto b = arrayAllocatorCast<std::uint16_t>(Utility::move(a));
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(a.data(), nullptr);
}

void GrowableArrayTest::castEmpty() {
    /* This models a typical use case -- an empty array is resized / reserved
       to a dynamic size, which is however 0 in that particular case. That
       results in no change being made to the array, it's still nullptr with
       the default deleter. */
    Array<std::uint16_t> a;
    arrayResize(a, 0);
    CORRADE_COMPARE(a.data(), nullptr);
    CORRADE_COMPARE(a.deleter(), nullptr);

    /* If we then want to cast the array to a different type, the behavior
       should not be different from what's in cast() above if the array is
       empty with the default deleter. In this case it should also just keep
       the default deleter.

       It *should definitely not* use a growable deleter in this case -- that
       deleter needs a non-null data array in order to store the capacity, and
       giving it null would make it blow up on the first capacity access. */
    auto b = arrayAllocatorCast<char>(Utility::move(a));
    CORRADE_COMPARE(b.size(), 0);
    CORRADE_COMPARE(b.data(), nullptr);
    CORRADE_COMPARE(b.deleter(), nullptr);
}

void GrowableArrayTest::castNonTrivial() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Array<char> a;
    arrayResize<char, ArrayNewAllocator<char>>(a, 10);

    Containers::String out;
    Error redirectError{&out};
    arrayAllocatorCast<std::uint16_t>(Utility::move(a));
    CORRADE_COMPARE(out,
        "Containers::arrayAllocatorCast(): the array has to use the ArrayMallocAllocator or a derivative\n");
}

void GrowableArrayTest::castNonGrowable() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Array<char> a{10};

    Containers::String out;
    Error redirectError{&out};
    arrayAllocatorCast<std::uint16_t>(Utility::move(a));
    CORRADE_COMPARE(out,
        "Containers::arrayAllocatorCast(): the array has to use the ArrayMallocAllocator or a derivative\n");
}

void GrowableArrayTest::castInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    Array<char> a;
    arrayResize(a, 10);

    Containers::String out;
    Error redirectError{&out};
    arrayAllocatorCast<std::uint32_t>(Utility::move(a));
    CORRADE_COMPARE(out,
        "Containers::arrayAllocatorCast(): can't reinterpret 10 1-byte items into a 4-byte type\n");
}

void GrowableArrayTest::explicitAllocatorParameter() {
    Array<int> a;
    arrayReserve<ArrayNewAllocator>(a, 10);
    /* Calling it again should be a no-op if it correctly checks for capacity */
    arrayReserve<ArrayNewAllocator>(a, 8);
    CORRADE_VERIFY(!arrayIsGrowable(a));
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
    CORRADE_COMPARE(arrayCapacity<ArrayNewAllocator>(a), 10);

    arrayClear<ArrayNewAllocator>(a);
    CORRADE_VERIFY(!arrayIsGrowable(a));
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
    CORRADE_COMPARE(arrayCapacity<ArrayNewAllocator>(a), 10);

    #ifdef CORRADE_BUILD_DEPRECATED /* (If this isn't called, it's fine) */
    CORRADE_IGNORE_DEPRECATED_PUSH
    arrayResize<ArrayNewAllocator>(a, Corrade::DefaultInit, 1);
    CORRADE_IGNORE_DEPRECATED_POP
    #endif
    arrayResize<ArrayNewAllocator>(a, Corrade::ValueInit, 2);
    arrayResize<ArrayNewAllocator>(a, 3);
    arrayResize<ArrayNewAllocator>(a, Corrade::NoInit, 4);
    arrayResize<ArrayNewAllocator>(a, Corrade::DirectInit, 5, 6);
    arrayResize<ArrayNewAllocator>(a, 5, 6);
    CORRADE_VERIFY(!arrayIsGrowable(a));
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
    CORRADE_COMPARE(a.size(), 5);

    const int six = 6;
    {
        int& value = arrayAppend<ArrayNewAllocator>(a, six);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(value, 6);
    } {
        int& value = arrayInsert<ArrayNewAllocator>(a, 0, six);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(value, 6);
    } {
        int& value = arrayAppend<ArrayNewAllocator>(a, Corrade::InPlaceInit, 7);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(value, 7);
    } {
        int& value = arrayInsert<ArrayNewAllocator>(a, 0, Corrade::InPlaceInit, 7);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(value, 7);
    } {
        Containers::ArrayView<int> view = arrayAppend<ArrayNewAllocator>(a, {8, 9, 10});
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 3);
        CORRADE_COMPARE(view[2], 10);
    } {
        Containers::ArrayView<int> view = arrayInsert<ArrayNewAllocator>(a, 0, {8, 9, 10});
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 3);
        CORRADE_COMPARE(view[2], 10);
    } {
        const int values[]{11, 12, 13};
        Containers::ArrayView<int> view = arrayAppend<ArrayNewAllocator>(a, arrayView(values));
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 3);
        CORRADE_COMPARE(view[1], 12);
    } {
        const int values[]{11, 12, 13};
        Containers::ArrayView<int> view = arrayInsert<ArrayNewAllocator>(a, 0, arrayView(values));
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 3);
        CORRADE_COMPARE(view[1], 12);
    } {
        Containers::ArrayView<int> view = arrayAppend<ArrayNewAllocator>(a, Corrade::ValueInit, 2);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 2);
        CORRADE_COMPARE(view[0], 0);
        CORRADE_COMPARE(view[1], 0);
        CORRADE_COMPARE(a[21], 0);
        CORRADE_COMPARE(a[22], 0);
    } {
        Containers::ArrayView<int> view = arrayAppend<ArrayNewAllocator>(a, Corrade::NoInit, 2);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 2);
        view[0] = 14;
        view[1] = 15;
        CORRADE_COMPARE(a[23], 14);
    } {
        Containers::ArrayView<int> view = arrayAppend<ArrayNewAllocator>(a, Corrade::DirectInit, 2, 16);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 2);
        CORRADE_COMPARE(view[0], 16);
        CORRADE_COMPARE(view[1], 16);
        CORRADE_COMPARE(a[25], 16);
    } {
        Containers::ArrayView<int> view = arrayInsert<ArrayNewAllocator>(a, 2, Corrade::ValueInit, 2);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 2);
        CORRADE_COMPARE(view[0], 0);
        CORRADE_COMPARE(view[1], 0);
        CORRADE_COMPARE(a[2], 0);
        CORRADE_COMPARE(a[3], 0);
    } {
        Containers::ArrayView<int> view = arrayInsert<ArrayNewAllocator>(a, 4, Corrade::NoInit, 2);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 2);
        view[0] = 17;
        view[1] = 18;
        CORRADE_COMPARE(a[5], 18);
    } {
        Containers::ArrayView<int> view = arrayInsert<ArrayNewAllocator>(a, 3, Corrade::DirectInit, 2, 19);
        CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
        CORRADE_COMPARE(view.size(), 2);
        CORRADE_COMPARE(view[0], 19);
        CORRADE_COMPARE(view[1], 19);
        CORRADE_COMPARE(a[3], 19);
        CORRADE_COMPARE(a[4], 19);
    }
    CORRADE_COMPARE(a.size(), 33);

    arrayRemove<ArrayNewAllocator>(a, 15);
    arrayRemoveUnordered<ArrayNewAllocator>(a, 15);
    arrayRemoveSuffix<ArrayNewAllocator>(a);
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
    /* We're not using the malloc allocator, which means these will be a no-op */
    arrayShrink<ArrayMallocAllocator>(a);
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
    arrayShrink<ArrayMallocAllocator>(a, Corrade::NoInit);
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
    #ifdef CORRADE_BUILD_DEPRECATED /* (If this isn't called, it's fine) */
    CORRADE_IGNORE_DEPRECATED_PUSH
    arrayShrink<ArrayMallocAllocator>(a, Corrade::DefaultInit);
    CORRADE_IGNORE_DEPRECATED_POP
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
    #endif
    arrayShrink<ArrayMallocAllocator>(a, Corrade::ValueInit);
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(a));
    /* After this it will finally lose the growable status */
    arrayShrink<ArrayNewAllocator>(a);
    CORRADE_VERIFY(!arrayIsGrowable<ArrayNewAllocator>(a));
    CORRADE_VERIFY(!a.deleter());
    CORRADE_COMPARE(a.size(), 30);

    /* Verifying that the variadic arguments are correctly forwarded */
    /** @todo use a different allocator here once it exists -- this one would
        be picked up implicitly as well so it doesn't really test anything */
    Array<Movable> b;
    arrayResize<ArrayNewAllocator>(b, Corrade::DirectInit, 5, Movable{6});
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(b));

    arrayAppend<ArrayNewAllocator>(b, Movable{1});
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(b));

    arrayInsert<ArrayNewAllocator>(b, 0, Movable{1});
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(b));

    arrayAppend<ArrayNewAllocator>(b, Corrade::InPlaceInit, 2);
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(b));

    arrayInsert<ArrayNewAllocator>(b, 0, Corrade::InPlaceInit, 2);
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(b));

    arrayAppend<ArrayNewAllocator>(b, Corrade::DirectInit, 2, Movable{1});
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(b));

    arrayInsert<ArrayNewAllocator>(b, 0, Corrade::DirectInit, 2, Movable{1});
    CORRADE_VERIFY(arrayIsGrowable<ArrayNewAllocator>(b));

    CORRADE_COMPARE(b.size(), 13);
}

void GrowableArrayTest::constructorExplicitInCopyInitialization() {
    /* See constructHelpers.h for details about this compiler-specific issue */
    struct ExplicitDefault {
        explicit ExplicitDefault() {}
    };

    /* The arrayResize(ValueInit) overload has a special case for
       non-trivially-constructible initialization that's affected by this
       issue as well, be sure to have it picked in this test. This check
       corresponds to the check in the code itself. */
    #ifdef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(!__has_trivial_constructor(ExplicitDefault));
    #else
    CORRADE_VERIFY(!std::is_trivially_constructible<ExplicitDefault>::value);
    #endif

    struct ContainingExplicitDefaultWithImplicitConstructor {
        ExplicitDefault a;
    };

    /* This alone works */
    ContainingExplicitDefaultWithImplicitConstructor a;
    static_cast<void>(a);

    /* So this should too */
    Containers::Array<ContainingExplicitDefaultWithImplicitConstructor> b;
    arrayResize(b, Corrade::DirectInit, 2);
    arrayResize(b, Corrade::ValueInit, 1);
    arrayAppend(b, Corrade::InPlaceInit);
    arrayInsert(b, 0, Corrade::InPlaceInit);
    CORRADE_COMPARE(b.size(), 3);
}

void GrowableArrayTest::copyConstructPlainStruct() {
    struct ExtremelyTrivial {
        int a;
        char b;
    };

    Array<ExtremelyTrivial> a;

    /* This needs special handling on GCC 4.8, where T{b} (copy-construction)
       attempts to convert ExtremelyTrivial to int to initialize the first
       argument and fails miserably. */
    ExtremelyTrivial value;
    arrayAppend(a, value);
    arrayInsert(a, 0, value);

    /* This copy-constructs the new values */
    arrayResize(a, Corrade::DirectInit, 10, ExtremelyTrivial{4, 'b'});

    /* And this also */
    const ExtremelyTrivial data[2]{
        {5, 'c'},
        {6, 'd'}
    };
    arrayAppend(a, arrayView(data));
    arrayInsert(a, 0, arrayView(data));

    CORRADE_COMPARE(a.size(), 14);
}

void GrowableArrayTest::moveConstructPlainStruct() {
    struct MoveOnlyStruct {
        int a;
        char c;
        Array<int> b;
    };

    Array<MoveOnlyStruct> a;

    /* This needs special handling on GCC 4.8, where T{Utility::move(b)}
       attempts to convert MoveOnlyStruct to int to initialize the first
       argument and fails miserably. */
    arrayAppend(a, Corrade::InPlaceInit, 3, 'a', nullptr);
    arrayAppend(a, Corrade::InPlaceInit, 4, 'b', nullptr);
    arrayAppend(a, Corrade::InPlaceInit, 5, 'c', nullptr);

    /* This is another case where move constructors get called */
    arrayResize(a, 15);

    /* Here a move constructor gets called indirectly as the args are forwarded
       to the InPlaceInit version. In this case there's a workaround for
       the constructorExplicitInCopyInitialization() case from above so we're
       just reusing that to mix in the 4.8-specific variant also */
    arrayAppend(a, MoveOnlyStruct{5, 'c', nullptr});

    /* Here a move constructor gets called on the last element to shift it
       forward and then a move constructor gets called to place the new
       element */
    arrayInsert(a, 16, MoveOnlyStruct{6, 'd', nullptr});

    CORRADE_COMPARE(a.size(), 17);
}

template<template<class> class> struct AllocatorName;
template<> struct AllocatorName<ArrayNewAllocator> {
    static const char* name() { return "ArrayNewAllocator"; }
};
template<> struct AllocatorName<ArrayMallocAllocator> {
    static const char* name() { return "ArrayMallocAllocator"; }
};

template<std::size_t alignment> struct alignas(alignment) Aligned {
    char foo;
};

template<template<class> class Allocator, std::size_t alignment> void GrowableArrayTest::allocationAlignment() {
    setTestCaseTemplateName({AllocatorName<Allocator>::name(), Utility::format("{}", alignment)});

    if(alignment > Implementation::DefaultAllocationAlignment)
        CORRADE_SKIP(alignment << Debug::nospace << "-byte alignment is larger than platform default allocation alignment, skipping");

    /* Should always fit std::size_t, at least */
    CORRADE_COMPARE_AS(Allocator<Aligned<alignment>>::AllocationOffset, sizeof(std::size_t),
        TestSuite::Compare::GreaterOrEqual);

    /* Offset should be aligned for the type */
    CORRADE_COMPARE_AS(Allocator<Aligned<alignment>>::AllocationOffset, alignment,
        TestSuite::Compare::Divisible);

    /* We're not stupid with the assumptions, hopefully */
    CORRADE_COMPARE(sizeof(Aligned<alignment>), alignof(Aligned<alignment>));

    /* All (re)allocations should be aligned */
    Array<Aligned<alignment>> a;
    for(std::size_t i = 0; i != 100; ++i) {
        CORRADE_ITERATION(i);
        arrayAppend<Allocator>(a, Corrade::InPlaceInit, 'a');
        CORRADE_COMPARE_AS(reinterpret_cast<std::uintptr_t>(a.data()), alignment,
            TestSuite::Compare::Divisible);
    }
}

void GrowableArrayTest::appendInsertConflictingType() {
    Array<unsigned> a;
    Array<unsigned> b{Corrade::InPlaceInit, {3}};

    /* If the second argument is just const T& or T&&, these would fail to
       compile due to a conflict in template type resolution (int vs unsigned).
       Has to be std::common_type<T>::type, which avoids the conflict and
       instead accepts anything implicitly convertible to T. */
    const int value = 5;
    arrayAppend(a, value);
    arrayAppend(a, 5);
    arrayAppend(a, b);
    arrayAppend(a, {4, 4});
    arrayAppend<ArrayAllocator>(a, value);
    arrayAppend<ArrayAllocator>(a, 5);
    arrayAppend<ArrayAllocator>(a, b);
    arrayAppend<ArrayAllocator>(a, {4, 4});
    arrayInsert(a, 0, value);
    arrayInsert(a, 0, 5);
    arrayInsert(a, 0, b);
    arrayInsert(a, 0, {4, 4});
    arrayInsert<ArrayAllocator>(a, 0, value);
    arrayInsert<ArrayAllocator>(a, 0, 5);
    arrayInsert<ArrayAllocator>(a, 0, b);
    arrayInsert<ArrayAllocator>(a, 0, {4, 4});
    CORRADE_COMPARE_AS(a, Containers::arrayView<unsigned>({
        4, 4, 3, 5, 5, 4, 4, 3, 5, 5, 5, 5, 3, 4, 4, 5, 5, 3, 4, 4
    }), TestSuite::Compare::Container);
}

void GrowableArrayTest::appendInsertArrayElement() {
    auto&& data = AppendInsertArrayElementData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    /* Extra unused capacity at the end, which should get checked as well */
    Array<int> a;
    if(data.growable) {
        arrayReserve(a, 20);
        arrayAppend(a, Corrade::NoInit, 10);

        /* These have no easy chance to assert, unfortunately */
        arrayAppend(a, Corrade::InPlaceInit, a[0]);
        arrayAppend(a, Corrade::InPlaceInit, Utility::move(a[0]));
        arrayInsert(a, 0, Corrade::InPlaceInit, a[0]);
        arrayInsert(a, 0, Corrade::InPlaceInit, Utility::move(a[0]));
    } else {
        a = Array<int>{Corrade::NoInit, 10};
    }

    Containers::String out;
    Error redirectError{&out};
    /* Beginning of the array */
    arrayAppend(a, a[0]);
    arrayAppend(a, Utility::move(a[0]));
    arrayInsert(a, 0, a[0]);
    arrayInsert(a, 0, Utility::move(a[0]));
    /* End of the array */
    arrayAppend(a, a[9]);
    arrayAppend(a, Utility::move(a[9]));
    arrayInsert(a, 0, a[9]);
    arrayInsert(a, 0, Utility::move(a[9]));
    /* End of the capacity. Using .data() to avoid tripping up on a range
       assert, if data would be actually read from there, ASan would
       complain. */
    arrayAppend(a, a.data()[data.capacityEnd]);
    arrayAppend(a, Utility::move(a.data()[data.capacityEnd]));
    arrayInsert(a, 0, a.data()[data.capacityEnd]);
    arrayInsert(a, 0, Utility::move(a.data()[data.capacityEnd]));
    CORRADE_COMPARE_AS(out,
        "Containers::arrayAppend(): use the list variant to append values from within the array itself\n"
        "Containers::arrayAppend(): use the list variant to append values from within the array itself\n"
        "Containers::arrayInsert(): use the list variant to insert values from within the array itself\n"
        "Containers::arrayInsert(): use the list variant to insert values from within the array itself\n"
        "Containers::arrayAppend(): use the list variant to append values from within the array itself\n"
        "Containers::arrayAppend(): use the list variant to append values from within the array itself\n"
        "Containers::arrayInsert(): use the list variant to insert values from within the array itself\n"
        "Containers::arrayInsert(): use the list variant to insert values from within the array itself\n"
        "Containers::arrayAppend(): use the list variant to append values from within the array itself\n"
        "Containers::arrayAppend(): use the list variant to append values from within the array itself\n"
        "Containers::arrayInsert(): use the list variant to insert values from within the array itself\n"
        "Containers::arrayInsert(): use the list variant to insert values from within the array itself\n",
        TestSuite::Compare::String);
}

template<bool growable> void GrowableArrayTest::appendInsertArraySlice() {
    auto&& data = AppendInsertArraySliceData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseName(growable ? CORRADE_FUNCTION : "appendInsertArraySliceNonGrowable");

    #ifdef _CORRADE_CONTAINERS_SANITIZER_ENABLED
    if(data.expectInvalidBegin != ~std::size_t{})
        CORRADE_SKIP("Copying from container capacity triggers a AddressSanitizer failure, skipping.");
    #endif

    /* Not using the malloc allocator because realloc() might sometimes grow
       the allocation, hiding the problem */
    Array<int> a;
    if(growable) {
        arrayAppend<ArrayNewAllocator>(a, {
            00, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120
        });
        arrayResize<ArrayNewAllocator>(a, 10);
    } else {
        /* The extra 3 items aren't included in the size, arrayCapacity()
           doesn't know about them either. GCC 4.8 gets extremely confused with
           just new int[]{...} without a size, have to explicitly say it. */
        a = Array<int>{new int[13]{00, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120}, 10};
    }
    int* prev = a.data();

    /* Not using a.slice() as the slice may be out of range */
    ArrayView<int> slice{a.data() + data.begin, 4};
    if(data.insert != ~std::size_t{})
        arrayInsert<ArrayNewAllocator>(a, data.insert, slice);
    else
        arrayAppend<ArrayNewAllocator>(a, slice);

    /* Verify the operation caused reallocation */
    CORRADE_VERIFY(a.data() != prev);

    if(data.expectInvalidBegin == ~std::size_t{})
        CORRADE_COMPARE_AS(a,
            Containers::arrayView(data.expected),
            TestSuite::Compare::Container);
    else {
        CORRADE_COMPARE_AS(a.prefix(data.expectInvalidBegin),
            Containers::arrayView(data.expected).prefix(data.expectInvalidBegin),
            TestSuite::Compare::Container);
        {
            CORRADE_EXPECT_FAIL("Garbage from container capacity is being copied.");
            CORRADE_COMPARE_AS(a.slice(data.expectInvalidBegin, data.expectInvalidEnd),
                Containers::arrayView(data.expected).slice(data.expectInvalidBegin, data.expectInvalidEnd),
                TestSuite::Compare::Container);
        }
        CORRADE_COMPARE_AS(a.exceptPrefix(data.expectInvalidEnd),
            Containers::arrayView(data.expected).exceptPrefix(data.expectInvalidEnd),
            TestSuite::Compare::Container);
    }
}

void GrowableArrayTest::insertArraySliceIntoItself() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    /* For arrayAppend() this happens as well if the slice is cutting into the
       container capacity, but that should be caught by ASan already so I
       decided to not handle it there, only in arrayInsert() */

    Array<int> a;
    arrayAppend(a, {00, 10, 20, 30, 40, 50, 60, 70, 80, 90});

    /* All these are okay as they're exactly on the bound, so no overlap */
    arrayInsert(a, 2, a.slice(2, 5));
    arrayInsert(a, 5, a.slice(2, 5));
    arrayInsert(a, 5, a.slice(5, 9));
    arrayInsert(a, 9, a.slice(5, 9));

    Containers::String out;
    Error redirectError{&out};
    arrayInsert(a, 3, a.slice(2, 5));
    arrayInsert(a, 4, a.slice(2, 5));
    arrayInsert(a, 6, a.slice(5, 9));
    arrayInsert(a, 8, a.slice(5, 9));
    CORRADE_COMPARE_AS(out,
        "Containers::arrayInsert(): attempting to insert a slice [2:5] into itself at index 3\n"
        "Containers::arrayInsert(): attempting to insert a slice [2:5] into itself at index 4\n"
        "Containers::arrayInsert(): attempting to insert a slice [5:9] into itself at index 6\n"
        "Containers::arrayInsert(): attempting to insert a slice [5:9] into itself at index 8\n",
        TestSuite::Compare::String);
}

void GrowableArrayTest::benchmarkAppendVector() {
    std::vector<Movable> vector;
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i)
            vector.emplace_back(i);
    }

    CORRADE_COMPARE(vector.size(), 1000000);
}

void GrowableArrayTest::benchmarkAppendArray() {
    Array<Movable> array;
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i)
            arrayAppend(array, Corrade::InPlaceInit, int(i));
    }

    CORRADE_COMPARE(array.size(), 1000000);
}

void GrowableArrayTest::benchmarkAppendReservedVector() {
    std::vector<Movable> vector;
    vector.reserve(1000000);
    Movable* data = vector.data();
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i)
            vector.emplace_back(i);
    }

    CORRADE_COMPARE(vector.size(), 1000000);
    CORRADE_COMPARE(vector.data(), data);
}

void GrowableArrayTest::benchmarkAppendReservedArray() {
    Array<Movable> array;
    arrayReserve(array, 1000000);
    Movable* data = array.data();
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i)
            arrayAppend(array, Corrade::InPlaceInit, int(i));
    }

    CORRADE_COMPARE(array.size(), 1000000);
    CORRADE_COMPARE(array.data(), data);
}

void GrowableArrayTest::benchmarkAppendTrivialVector() {
    std::vector<int> vector;
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i)
            vector.push_back(i);
    }

    CORRADE_COMPARE(vector.size(), 1000000);
}

template<template<class> class Allocator> void GrowableArrayTest::benchmarkAppendTrivialArray() {
    setTestCaseTemplateName(AllocatorName<Allocator>::name());

    Array<int> array;
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i)
            arrayAppend<int, Allocator<int>>(array, int(i));
    }

    CORRADE_COMPARE(array.size(), 1000000);
}

void GrowableArrayTest::benchmarkAppendTrivialReservedVector() {
    std::vector<int> vector;
    vector.reserve(1000000);
    int* data = vector.data();
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i)
            vector.push_back(i);
    }

    CORRADE_COMPARE(vector.size(), 1000000);
    CORRADE_COMPARE(vector.data(), data);
}

void GrowableArrayTest::benchmarkAppendTrivialReservedArray() {
    Array<int> array;
    arrayReserve(array, 1000000);
    int* data = array.data();
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i)
            arrayAppend(array, int(i));
    }

    CORRADE_COMPARE(array.size(), 1000000);
    CORRADE_COMPARE(array.data(), data);
}

void GrowableArrayTest::benchmarkAppendBatchTrivialVector() {
    std::vector<int> vector;
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; i += 10)
            vector.insert(vector.end(), {
                int(i),
                int(i) + 1,
                int(i) + 2,
                int(i) + 3,
                int(i) + 4,
                int(i) + 5,
                int(i) + 6,
                int(i) + 7,
                int(i) + 8,
                int(i) + 9});
    }

    CORRADE_COMPARE(vector.size(), 1000000);
}

template<template<class> class Allocator> void GrowableArrayTest::benchmarkAppendBatchTrivialArray() {
    setTestCaseTemplateName(AllocatorName<Allocator>::name());

    Array<int> array;
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; i += 10)
            arrayAppend<int, Allocator<int>>(array, {
                int(i),
                int(i) + 1,
                int(i) + 2,
                int(i) + 3,
                int(i) + 4,
                int(i) + 5,
                int(i) + 6,
                int(i) + 7,
                int(i) + 8,
                int(i) + 9});
    }

    CORRADE_COMPARE(array.size(), 1000000);
}

void GrowableArrayTest::benchmarkAppendBatchTrivialReservedVector() {
    std::vector<int> vector;
    vector.reserve(1000000);
    int* data = vector.data();
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; i += 10)
            vector.insert(vector.end(), {
                int(i),
                int(i) + 1,
                int(i) + 2,
                int(i) + 3,
                int(i) + 4,
                int(i) + 5,
                int(i) + 6,
                int(i) + 7,
                int(i) + 8,
                int(i) + 9});
    }

    CORRADE_COMPARE(vector.size(), 1000000);
    CORRADE_COMPARE(vector.data(), data);
}

void GrowableArrayTest::benchmarkAppendBatchTrivialReservedArray() {
    Array<int> array;
    arrayReserve(array, 1000000);
    int* data = array.data();
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; i += 10)
            arrayAppend(array, {
                int(i),
                int(i) + 1,
                int(i) + 2,
                int(i) + 3,
                int(i) + 4,
                int(i) + 5,
                int(i) + 6,
                int(i) + 7,
                int(i) + 8,
                int(i) + 9});
    }

    CORRADE_COMPARE(array.size(), 1000000);
    CORRADE_COMPARE(array.data(), data);
}

const char* AllocationBenchmarkName[] {
    "allocations",
    "allocation reuse",
    "reallocations"
};

std::size_t allocationCount, allocationReuseCount, reallocationCount;

void GrowableArrayTest::benchmarkAllocationsBegin() {
    allocationCount = 0;
    allocationReuseCount = 0;
    reallocationCount = 0;
}

std::uint64_t GrowableArrayTest::benchmarkAllocationsEnd() {
    if(testCaseInstanceId() == 0)
        return allocationCount;
    else if(testCaseInstanceId() == 1)
        return allocationReuseCount;
    else if(testCaseInstanceId() == 2)
        return reallocationCount;
    else CORRADE_INTERNAL_ASSERT_UNREACHABLE();
}

void GrowableArrayTest::benchmarkAllocationsVector() {
    setTestCaseDescription(AllocationBenchmarkName[testCaseInstanceId()]);

    std::vector<int> vector;
    int *prevData = nullptr;
    std::size_t prevCapacity = 0;
    std::set<int*> used;
    Debug capacities{testCaseInstanceId() ? nullptr : Debug::output()};
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i) {
            vector.push_back(i);
            if(vector.data() != prevData) {
                ++allocationCount;
                capacities << vector.capacity();
                if(used.count(vector.data())) {
                    ++allocationReuseCount;
                    capacities << Debug::nospace << "@";
                }
            } else if(vector.capacity() != prevCapacity) {
                ++reallocationCount;
                capacities << vector.capacity() << Debug::nospace << "!";
            }
            prevData = vector.data();
            prevCapacity = vector.capacity();
            used.insert(vector.data());
        }
    }

    CORRADE_COMPARE(vector.size(), 1000000);
}

template<template<class> class Allocator> void GrowableArrayTest::benchmarkAllocationsArray() {
    setTestCaseTemplateName(AllocatorName<Allocator>::name());
    setTestCaseDescription(AllocationBenchmarkName[testCaseInstanceId()]);

    Array<int> array;
    int *prevData = nullptr;
    std::size_t prevCapacity = 0;
    std::set<int*> used;
    Debug capacities{testCaseInstanceId() ? nullptr : Debug::output()};
    CORRADE_BENCHMARK(1) {
        for(std::size_t i = 0; i != 1000000; ++i) {
            arrayAppend<int, Allocator<int>>(array, int(i));
            std::size_t capacity = arrayCapacity<int, Allocator<int>>(array);
            if(array.data() != prevData) {
                ++allocationCount;
                capacities << capacity;
                if(used.count(array.data())) {
                    ++allocationReuseCount;
                    capacities << Debug::nospace << "@";
                }
            } else if(capacity != prevCapacity) {
                ++reallocationCount;
                capacities << capacity << Debug::nospace << "!";
            }
            prevData = array.data();
            prevCapacity = capacity;
            used.insert(array.data());
        }
    }

    CORRADE_COMPARE(array.size(), 1000000);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::GrowableArrayTest)
