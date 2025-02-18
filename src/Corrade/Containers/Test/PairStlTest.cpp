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

#include "Corrade/Containers/PairStl.h"
#include "Corrade/TestSuite/Tester.h"

namespace Corrade { namespace Containers { namespace Test { namespace {

struct PairStlTest: TestSuite::Tester {
    explicit PairStlTest();

    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    void triviallyCopyable();
    void triviallyMovable();
    #endif

    void convertCopy();
    void convertMove();
};

PairStlTest::PairStlTest() {
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    addTests({&PairStlTest::triviallyCopyable,
              &PairStlTest::triviallyMovable});
    #endif

    addTests({&PairStlTest::convertCopy,
              &PairStlTest::convertMove});
}

#ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
void PairStlTest::triviallyCopyable() {
    /* Part of PairTest::copy() expanded to compare with std::pair */

    CORRADE_VERIFY(std::is_trivially_copy_constructible<Pair<float, int>>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<Pair<float, int>>::value);
    CORRADE_VERIFY(std::is_trivially_copyable<Pair<float, int>>::value);

    CORRADE_VERIFY(std::is_trivially_copy_constructible<std::pair<float, int>>::value);
    {
        CORRADE_EXPECT_FAIL("std::pair is not trivially copy assignable.");
        CORRADE_VERIFY(std::is_trivially_copy_assignable<std::pair<float, int>>::value);
    } {
        #if !(defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG) && _MSC_VER >= 1920) && !(defined(CORRADE_TARGET_CLANG_CL) && __clang_major__ >= 15)
        /* MSVC 2019 says this is copyable but not copy assignable, older
           versions are consistent with libc++ and libstdc++. Bug? Clang-cl 15
           *also* seems to think it's trivially copyable (but not assignable),
           so I guess it's something fishy inside MSVC STL? */
        CORRADE_EXPECT_FAIL("std::pair is not trivially copyable except for MSVC 2019 and newer.");
        #endif
        CORRADE_VERIFY(std::is_trivially_copyable<std::pair<float, int>>::value);
    }
}

void PairStlTest::triviallyMovable() {
    /* Part of PairTest::move() expanded to compare with std::pair */

    CORRADE_VERIFY(std::is_trivially_move_constructible<Pair<float, int>>::value);
    CORRADE_VERIFY(std::is_trivially_move_assignable<Pair<float, int>>::value);

    CORRADE_VERIFY(std::is_trivially_move_constructible<std::pair<float, int>>::value);
    {
        CORRADE_EXPECT_FAIL("std::pair is not trivially move assignable.");
        CORRADE_VERIFY(std::is_trivially_move_assignable<std::pair<float, int>>::value);
    }
}
#endif

void PairStlTest::convertCopy() {
    std::pair<float, int> a{35.0f, 4};

    Pair<float, int> b = a;
    CORRADE_COMPARE(b.first(), 35.0f);
    CORRADE_COMPARE(b.second(), 4);

    std::pair<float, int> c = b;
    CORRADE_COMPARE(c.first, 35.0f);
    CORRADE_COMPARE(c.second, 4);

    auto d = pair(c);
    CORRADE_VERIFY(std::is_same<decltype(d), Pair<float, int>>::value);
    CORRADE_COMPARE(d.first(), 35.0f);
    CORRADE_COMPARE(d.second(), 4);
}

void PairStlTest::convertMove() {
    std::pair<Pointer<float>, Pointer<int>> a{pointer(new float{35.0f}), pointer(new int{4})};

    Pair<Pointer<float>, Pointer<int>> b = Utility::move(a);
    CORRADE_COMPARE(*b.first(), 35.0f);
    CORRADE_COMPARE(*b.second(), 4);
    CORRADE_VERIFY(!a.first);
    CORRADE_VERIFY(!a.second);

    std::pair<Pointer<float>, Pointer<int>> c = Utility::move(b);
    CORRADE_COMPARE(*c.first, 35.0f);
    CORRADE_COMPARE(*c.second, 4);
    CORRADE_VERIFY(!b.first());
    CORRADE_VERIFY(!b.second());

    auto d = pair(Utility::move(c));
    CORRADE_VERIFY(std::is_same<decltype(d), Pair<Pointer<float>, Pointer<int>>>::value);
    CORRADE_COMPARE(*d.first(), 35.0f);
    CORRADE_COMPARE(*d.second(), 4);
    CORRADE_VERIFY(!c.first);
    CORRADE_VERIFY(!c.second);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::PairStlTest)
