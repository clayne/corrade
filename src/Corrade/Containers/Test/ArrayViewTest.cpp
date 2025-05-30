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

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/TestSuite/Tester.h"

namespace {

struct IntView {
    IntView(int* data, std::size_t size): data{data}, size{size} {}

    int* data;
    std::size_t size;
};

struct ConstIntView {
    constexpr ConstIntView(const int* data, std::size_t size): data{data}, size{size} {}

    const int* data;
    std::size_t size;
};

}

namespace Corrade { namespace Containers {

namespace Implementation {

template<> struct ArrayViewConverter<int, IntView> {
    /* Needed only by convertVoidFromExternalView() */
    static ArrayView<int> from(IntView other) {
        return {other.data, other.size};
    }
};

template<> struct ErasedArrayViewConverter<IntView>: ArrayViewConverter<int, IntView> {};
template<> struct ErasedArrayViewConverter<const IntView>: ArrayViewConverter<int, IntView> {};

template<> struct ArrayViewConverter<const int, ConstIntView> {
    constexpr static ArrayView<const int> from(ConstIntView other) {
        return {other.data, other.size};
    }

    constexpr static ConstIntView to(ArrayView<const int> other) {
        return {other.data(), other.size()};
    }
};

template<> struct ErasedArrayViewConverter<ConstIntView>: ArrayViewConverter<const int, ConstIntView> {};
template<> struct ErasedArrayViewConverter<const ConstIntView>: ArrayViewConverter<const int, ConstIntView> {};

/* To keep the ArrayView API in reasonable bounds, the const-adding variants
   have to be implemented explicitly */
template<> struct ArrayViewConverter<const int, IntView> {
    static ArrayView<const int> from(IntView other) {
        return {other.data, other.size};
    }
};
template<> struct ArrayViewConverter<int, ConstIntView> {
    constexpr static ConstIntView to(ArrayView<int> other) {
        return {other.data(), other.size()};
    }
};

}

namespace Test { namespace {

struct ArrayViewTest: TestSuite::Tester {
    explicit ArrayViewTest();

    void constructDefault();
    void constructDefaultVoid();
    void constructDefaultConstVoid();
    void construct();
    void constructVoid();
    void constructConstVoid();
    void constructVoidFrom();
    void constructConstVoidFrom();
    void constructConstVoidFromVoid();
    void constructNullptrSize();
    void constructFixedSize();
    void constructFixedSizeVoid();
    void constructFixedSizeConstVoid();
    void constructFromStatic();
    void constructFromStaticVoid();
    void constructFromStaticConstVoid();
    void constructDerived();
    void constructCopy();
    void constructInitializerList();

    void constructZeroNullPointerAmbiguity();
    void constructZeroNullPointerAmbiguityVoid();
    void constructZeroNullPointerAmbiguityConstVoid();

    void convertBool();
    void convertPointer();
    void convertConst();
    void convertExternalView();
    void convertConstFromExternalView();
    void convertToConstExternalView();
    void convertVoidFromExternalView();
    void convertConstVoidFromExternalView();
    void convertConstVoidFromConstExternalView();

    void access();
    void accessConst();
    void accessVoid();
    void accessConstVoid();
    void accessInvalid();
    void rangeBasedFor();

    void sliceInvalid();
    void sliceNullptr();
    void slice();
    void slicePointer();
    void sliceToStatic();
    void sliceToStaticPointer();
    void sliceZeroNullPointerAmbiguity();

    void cast();
    void castInvalid();
    void size();
};

typedef Containers::ArrayView<int> ArrayView;
typedef Containers::ArrayView<const int> ConstArrayView;
typedef Containers::ArrayView<void> VoidArrayView;
typedef Containers::ArrayView<const void> ConstVoidArrayView;

ArrayViewTest::ArrayViewTest() {
    addTests({&ArrayViewTest::constructDefault,
              &ArrayViewTest::constructDefaultVoid,
              &ArrayViewTest::constructDefaultConstVoid,
              &ArrayViewTest::construct,
              &ArrayViewTest::constructVoid,
              &ArrayViewTest::constructConstVoid,
              &ArrayViewTest::constructVoidFrom,
              &ArrayViewTest::constructConstVoidFrom,
              &ArrayViewTest::constructConstVoidFromVoid,
              &ArrayViewTest::constructNullptrSize,
              &ArrayViewTest::constructFixedSize,
              &ArrayViewTest::constructFixedSizeVoid,
              &ArrayViewTest::constructFixedSizeConstVoid,
              &ArrayViewTest::constructFromStatic,
              &ArrayViewTest::constructFromStaticVoid,
              &ArrayViewTest::constructFromStaticConstVoid,
              &ArrayViewTest::constructDerived,
              &ArrayViewTest::constructCopy,
              &ArrayViewTest::constructInitializerList,

              &ArrayViewTest::constructZeroNullPointerAmbiguity,
              &ArrayViewTest::constructZeroNullPointerAmbiguityVoid,
              &ArrayViewTest::constructZeroNullPointerAmbiguityConstVoid,

              &ArrayViewTest::convertBool,
              &ArrayViewTest::convertPointer,
              &ArrayViewTest::convertConst,
              &ArrayViewTest::convertExternalView,
              &ArrayViewTest::convertConstFromExternalView,
              &ArrayViewTest::convertToConstExternalView,
              &ArrayViewTest::convertVoidFromExternalView,
              &ArrayViewTest::convertConstVoidFromExternalView,
              &ArrayViewTest::convertConstVoidFromConstExternalView,

              &ArrayViewTest::access,
              &ArrayViewTest::accessConst,
              &ArrayViewTest::accessVoid,
              &ArrayViewTest::accessConstVoid,
              &ArrayViewTest::accessInvalid,
              &ArrayViewTest::rangeBasedFor,

              &ArrayViewTest::sliceInvalid,
              &ArrayViewTest::sliceNullptr,
              &ArrayViewTest::slice,
              &ArrayViewTest::slicePointer,
              &ArrayViewTest::sliceToStatic,
              &ArrayViewTest::sliceToStaticPointer,
              &ArrayViewTest::sliceZeroNullPointerAmbiguity,

              &ArrayViewTest::cast,
              &ArrayViewTest::castInvalid,
              &ArrayViewTest::size});
}

void ArrayViewTest::constructDefault() {
    ArrayView a;
    ArrayView b = nullptr;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(b == nullptr);
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_VERIFY(b.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 0);

    constexpr ArrayView ca;
    constexpr ArrayView cb = nullptr;
    constexpr void* dataA = ca.data();
    constexpr void* dataB = cb.data();
    constexpr bool emptyA = ca.isEmpty();
    constexpr bool emptyB = cb.isEmpty();
    constexpr std::size_t sizeA = ca.size();
    constexpr std::size_t sizeB = cb.size();
    CORRADE_VERIFY(dataA == nullptr);
    CORRADE_VERIFY(dataB == nullptr);
    CORRADE_VERIFY(emptyA);
    CORRADE_VERIFY(emptyB);
    CORRADE_COMPARE(sizeA, 0);
    CORRADE_COMPARE(sizeB, 0);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<ArrayView>::value);
}

void ArrayViewTest::constructDefaultVoid() {
    VoidArrayView a;
    VoidArrayView b = nullptr;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(b == nullptr);
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_VERIFY(b.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 0);

    constexpr VoidArrayView ca;
    constexpr VoidArrayView cb = nullptr;
    constexpr void* dataA = ca.data();
    constexpr void* dataB = cb.data();
    constexpr bool emptyA = ca.isEmpty();
    constexpr bool emptyB = cb.isEmpty();
    constexpr std::size_t sizeA = ca.size();
    constexpr std::size_t sizeB = cb.size();
    CORRADE_VERIFY(dataA == nullptr);
    CORRADE_VERIFY(dataB == nullptr);
    CORRADE_VERIFY(emptyA);
    CORRADE_VERIFY(emptyB);
    CORRADE_COMPARE(sizeA, 0);
    CORRADE_COMPARE(sizeB, 0);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<VoidArrayView>::value);
}

void ArrayViewTest::constructDefaultConstVoid() {
    ConstVoidArrayView a;
    ConstVoidArrayView b = nullptr;
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(b == nullptr);
    CORRADE_VERIFY(a.isEmpty());
    CORRADE_VERIFY(b.isEmpty());
    CORRADE_COMPARE(a.size(), 0);
    CORRADE_COMPARE(b.size(), 0);

    constexpr ConstVoidArrayView ca;
    constexpr ConstVoidArrayView cb = nullptr;
    constexpr const void* dataA = ca.data();
    constexpr const void* dataB = cb.data();
    constexpr bool emptyA = ca.isEmpty();
    constexpr bool emptyB = cb.isEmpty();
    constexpr std::size_t sizeA = ca.size();
    constexpr std::size_t sizeB = cb.size();
    CORRADE_VERIFY(dataA == nullptr);
    CORRADE_VERIFY(dataB == nullptr);
    CORRADE_VERIFY(emptyA);
    CORRADE_VERIFY(emptyB);
    CORRADE_COMPARE(sizeA, 0);
    CORRADE_COMPARE(sizeB, 0);

    CORRADE_VERIFY(std::is_nothrow_default_constructible<ConstVoidArrayView>::value);
}

/* Needs to be here in order to use it in constexpr */
constexpr int Array30[30]{};

void ArrayViewTest::construct() {
    int a[30];

    {
        ArrayView b = {a, 20};
        CORRADE_VERIFY(b == a);
        CORRADE_VERIFY(!b.isEmpty());
        CORRADE_COMPARE(b.size(), 20);
    } {
        auto b = arrayView(a, 20);
        CORRADE_VERIFY(std::is_same<decltype(b), ArrayView>::value);
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 20);

        auto c = arrayView(b);
        CORRADE_VERIFY(std::is_same<decltype(c), ArrayView>::value);
        CORRADE_VERIFY(c == a);
        CORRADE_COMPARE(c.size(), 20);
    }

    {
        constexpr ConstArrayView b = {Array30, 20};
        constexpr const void* data = b.data();
        constexpr bool empty = b.isEmpty();
        constexpr std::size_t size = b.size();
        CORRADE_COMPARE(data, Array30);
        CORRADE_VERIFY(!empty);
        CORRADE_COMPARE(size, 20);
    } {
        constexpr auto b = arrayView(Array30, 20);
        CORRADE_VERIFY(std::is_same<decltype(b), const ConstArrayView>::value);
        CORRADE_VERIFY(b == Array30);
        CORRADE_COMPARE(b.size(), 20);

        constexpr auto c = arrayView(b);
        CORRADE_VERIFY(std::is_same<decltype(c), const ConstArrayView>::value);
        CORRADE_VERIFY(c == Array30);
        CORRADE_COMPARE(c.size(), 20);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<ArrayView, int*, std::size_t>::value);
}

void ArrayViewTest::constructVoid() {
    void* a = reinterpret_cast<void*>(std::size_t(0xdeadbeef));
    VoidArrayView b(a, 25);
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 25);

    int* c = reinterpret_cast<int*>(std::size_t(0xdeadbeef));
    VoidArrayView d(c, 25);
    CORRADE_VERIFY(d == c);
    CORRADE_VERIFY(!d.isEmpty());
    CORRADE_COMPARE(d.size(), 100);

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidArrayView, int*, std::size_t>::value);
}

void ArrayViewTest::constructConstVoid() {
    void* a = reinterpret_cast<void*>(std::size_t(0xdeadbeef));
    ConstVoidArrayView b(a, 25);
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 25);

    int* c = reinterpret_cast<int*>(std::size_t(0xdeadbeef));
    ConstVoidArrayView d(c, 25);
    CORRADE_VERIFY(d == c);
    CORRADE_VERIFY(!d.isEmpty());
    CORRADE_COMPARE(d.size(), 100);

    constexpr ConstVoidArrayView cd{Array30, 25};
    constexpr const void* data = cd.data();
    constexpr bool empty = cd.isEmpty();
    constexpr std::size_t size = cd.size();
    CORRADE_COMPARE(data, Array30);
    CORRADE_VERIFY(!empty);
    CORRADE_COMPARE(size, 100);

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidArrayView, int*, std::size_t>::value);
}

void ArrayViewTest::constructVoidFrom() {
    int a[13];
    const ArrayView b = a;
    VoidArrayView c = b;
    CORRADE_VERIFY(c == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidArrayView, ArrayView>::value);
}

void ArrayViewTest::constructConstVoidFrom() {
    int a[13];
    const ArrayView b = a;
    const ConstArrayView cb = a;
    ConstVoidArrayView c = b;
    ConstVoidArrayView cc = cb;
    CORRADE_VERIFY(c == b);
    CORRADE_VERIFY(cc == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));
    CORRADE_COMPARE(cc.size(), 13*sizeof(int));

    constexpr ConstArrayView ccb = Array30;
    constexpr ConstVoidArrayView ccc = ccb;
    CORRADE_VERIFY(ccc == Array30);
    CORRADE_COMPARE(ccc.size(), 30*sizeof(int));

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidArrayView, ArrayView>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidArrayView, ConstArrayView>::value);
}

void ArrayViewTest::constructConstVoidFromVoid() {
    int a[13];
    const ArrayView b = a;
    VoidArrayView c = b;
    ConstVoidArrayView cc = c;
    CORRADE_VERIFY(c == b);
    CORRADE_VERIFY(cc == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));
    CORRADE_COMPARE(cc.size(), 13*sizeof(int));

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidArrayView, VoidArrayView>::value);
}

void ArrayViewTest::constructNullptrSize() {
    /* This should be allowed for e.g. just allocating memory in
       Magnum::GL::Buffer::setData() without passing any actual data */

    ArrayView a{nullptr, 5};
    CORRADE_VERIFY(a == nullptr);
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(a.size(), 5);

    constexpr ArrayView ca{nullptr, 5};
    CORRADE_VERIFY(ca == nullptr);
    CORRADE_VERIFY(!a.isEmpty());
    CORRADE_COMPARE(ca.size(), 5);
}

/* Needs to be here in order to use it in constexpr */
constexpr int Array13[13]{};

void ArrayViewTest::constructFixedSize() {
    int a[13];

    {
        ArrayView b = a;
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    } {
        auto b = arrayView(a);
        CORRADE_VERIFY(std::is_same<decltype(b), ArrayView>::value);
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    }

    {
        constexpr ConstArrayView b = Array13;
        CORRADE_VERIFY(b == Array13);
        CORRADE_COMPARE(b.size(), 13);
    } {
        constexpr auto b = arrayView(Array13);
        CORRADE_VERIFY(std::is_same<decltype(b), const ConstArrayView>::value);
        CORRADE_VERIFY(b == Array13);
        CORRADE_COMPARE(b.size(), 13);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<ArrayView, int(&)[10]>::value);
    /* Implicit construction from pointer should not be allowed */
    CORRADE_VERIFY(!std::is_convertible<int*, ArrayView>::value);
}

void ArrayViewTest::constructFixedSizeVoid() {
    int a[13];
    VoidArrayView b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 13*sizeof(int));

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidArrayView, int(&)[10]>::value);
}

void ArrayViewTest::constructFixedSizeConstVoid() {
    const int a[13]{};
    ConstVoidArrayView b = a;
    CORRADE_VERIFY(b == a);
    CORRADE_VERIFY(!b.isEmpty());
    CORRADE_COMPARE(b.size(), 13*sizeof(int));

    constexpr ConstVoidArrayView cb = Array30;
    CORRADE_VERIFY(cb == Array30);
    CORRADE_VERIFY(!cb.isEmpty());
    CORRADE_COMPARE(cb.size(), 30*sizeof(int));

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidArrayView, int(&)[10]>::value);
}

void ArrayViewTest::constructFromStatic() {
    int a[13];
    StaticArrayView<13, int> av = a;
    constexpr StaticArrayView<13, const int> cav = Array13;

    {
        ArrayView b = av;
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    } {
        auto b = arrayView(av);
        CORRADE_VERIFY(std::is_same<decltype(b), ArrayView>::value);
        CORRADE_VERIFY(b == a);
        CORRADE_COMPARE(b.size(), 13);
    }

    {
        constexpr ConstArrayView b = cav;
        CORRADE_VERIFY(b == cav);
        CORRADE_COMPARE(b.size(), 13);
    } {
        constexpr auto b = arrayView(cav);
        CORRADE_VERIFY(std::is_same<decltype(b), const ConstArrayView>::value);
        CORRADE_VERIFY(b == cav);
        CORRADE_COMPARE(b.size(), 13);
    }

    CORRADE_VERIFY(std::is_nothrow_constructible<ArrayView, StaticArrayView<10, int>>::value);
}

void ArrayViewTest::constructFromStaticVoid() {
    int a[13];
    const StaticArrayView<13, int> b = a;
    VoidArrayView c = b;
    CORRADE_VERIFY(c == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));

    /** @todo constexpr but not const? c++14? */

    CORRADE_VERIFY(std::is_nothrow_constructible<VoidArrayView, StaticArrayView<10, int>>::value);
}

void ArrayViewTest::constructFromStaticConstVoid() {
    int a[13];
    const StaticArrayView<13, int> b = a;
    const StaticArrayView<13, const int> cb = a;
    ConstVoidArrayView c = b;
    ConstVoidArrayView cc = cb;
    CORRADE_VERIFY(c == b);
    CORRADE_VERIFY(cc == b);
    CORRADE_COMPARE(c.size(), 13*sizeof(int));
    CORRADE_COMPARE(cc.size(), 13*sizeof(int));

    constexpr StaticArrayView<13, const int> ccb = Array13;
    constexpr ConstVoidArrayView ccc = ccb;
    CORRADE_VERIFY(ccc == Array13);
    CORRADE_COMPARE(ccc.size(), 13*sizeof(int));

    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidArrayView, StaticArrayView<10, int>>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<ConstVoidArrayView, StaticArrayView<10, const int>>::value);
}

/* Needs to be here in order to use it in constexpr */
struct Base {
    constexpr Base(): i{} {}
    int i;
};
struct Derived: Base {
    constexpr Derived() {}
};
constexpr Derived DerivedArray[5]
    /* This missing makes MSVC2015 complain it's not constexpr, but if present
       then GCC 4.8 fails to build. Eh. ¯\_(ツ)_/¯ */
    #ifdef CORRADE_MSVC2015_COMPATIBILITY
    {}
    #endif
    ;

void ArrayViewTest::constructDerived() {
    /* Valid use case: constructing Containers::ArrayView<Math::Vector<3, Float>>
       from Containers::ArrayView<Color3> because the data have the same size
       and data layout */

    Derived b[5];
    Containers::ArrayView<Derived> bv{b};
    Containers::ArrayView<Base> a{b};
    Containers::ArrayView<Base> av{bv};

    CORRADE_VERIFY(a == &b[0]);
    CORRADE_VERIFY(av == &b[0]);
    CORRADE_COMPARE(a.size(), 5);
    CORRADE_COMPARE(av.size(), 5);

    constexpr Containers::ArrayView<const Derived> cbv{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::ArrayView<const Base> ca{DerivedArray};
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr /* Implicit pointer downcast not constexpr on MSVC 2015 */
    #endif
    Containers::ArrayView<const Base> cav{cbv};

    CORRADE_VERIFY(ca == &DerivedArray[0]);
    CORRADE_VERIFY(cav == &DerivedArray[0]);
    CORRADE_COMPARE(ca.size(), 5);
    CORRADE_COMPARE(cav.size(), 5);

    CORRADE_VERIFY(std::is_nothrow_constructible<Containers::ArrayView<Base>, Derived(&)[5]>::value);
    CORRADE_VERIFY(std::is_nothrow_constructible<Containers::ArrayView<Base>, Containers::ArrayView<Derived>>::value);
}

void ArrayViewTest::constructCopy() {
    int data[30];
    ArrayView a{data, 20};

    ArrayView b = a;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 20);

    int data2[3];
    ArrayView c{data2};
    c = b;
    CORRADE_COMPARE(c.data(), &data[0]);
    CORRADE_COMPARE(c.size(), 20);

    CORRADE_VERIFY(std::is_copy_constructible<ArrayView>::value);
    CORRADE_VERIFY(std::is_copy_assignable<ArrayView>::value);
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copy_constructible<ArrayView>::value);
    CORRADE_VERIFY(std::is_trivially_copy_assignable<ArrayView>::value);
    #endif
    CORRADE_VERIFY(std::is_nothrow_copy_constructible<ArrayView>::value);
    CORRADE_VERIFY(std::is_nothrow_copy_assignable<ArrayView>::value);
}

void ArrayViewTest::constructInitializerList() {
    std::initializer_list<int> a = {3, 5, 7};
    ConstArrayView b = arrayView(a);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b.back(), 7);

    /* R-value init list should work too */
    CORRADE_COMPARE(arrayView<int>({3, 5, 7}).front(), 3);
}

/* Without a corresponding SFINAE check in the std::nullptr_t constructor, this
   is ambiguous, but *only* if the size_t overload has a second 64-bit
   argument. If both would be the same, it wouldn't be ambigous, if the size_t
   overload second argument was 32-bit and the other 16-bit it wouldn't be
   either. */
int integerArrayOverload(std::size_t, long long) {
    return 76;
}
int integerArrayOverload(ConstArrayView, int) {
    return 39;
}

void ArrayViewTest::constructZeroNullPointerAmbiguity() {
    /* Obvious cases */
    CORRADE_COMPARE(integerArrayOverload(25, 2), 76);
    CORRADE_COMPARE(integerArrayOverload(nullptr, 2), 39);

    /* This should pick the integer overload, not convert 0 to nullptr */
    CORRADE_COMPARE(integerArrayOverload(0, 3), 76);
}

/* Same as above, just for the void ArrayView specialization */
int integerArrayOverloadVoid(std::size_t, long long) {
    return 67;
}
int integerArrayOverloadVoid(VoidArrayView, int) {
    return 93;
}

void ArrayViewTest::constructZeroNullPointerAmbiguityVoid() {
    /* Obvious cases */
    CORRADE_COMPARE(integerArrayOverloadVoid(25, 2), 67);
    CORRADE_COMPARE(integerArrayOverloadVoid(nullptr, 2), 93);

    /* This should pick the integer overload, not convert 0 to nullptr */
    CORRADE_COMPARE(integerArrayOverloadVoid(0, 3), 67);
}

/* Same as above, just for the const void ArrayView specialization */
int integerArrayOverloadConstVoid(std::size_t, long long) {
    return 676;
}
int integerArrayOverloadConstVoid(ConstVoidArrayView, int) {
    return 939;
}

void ArrayViewTest::constructZeroNullPointerAmbiguityConstVoid() {
    /* Obvious cases */
    CORRADE_COMPARE(integerArrayOverloadConstVoid(25, 2), 676);
    CORRADE_COMPARE(integerArrayOverloadConstVoid(nullptr, 2), 939);

    /* This should pick the integer overload, not convert 0 to nullptr */
    CORRADE_COMPARE(integerArrayOverloadConstVoid(0, 3), 676);
}

void ArrayViewTest::convertBool() {
    int a[7];
    CORRADE_VERIFY(ArrayView(a));
    CORRADE_VERIFY(!ArrayView());
    CORRADE_VERIFY(VoidArrayView(a));
    CORRADE_VERIFY(!VoidArrayView());
    CORRADE_VERIFY(ConstVoidArrayView(a));
    CORRADE_VERIFY(!ConstVoidArrayView());

    constexpr ConstArrayView cb = Array30;
    constexpr bool boolCb = !!cb;
    CORRADE_VERIFY(boolCb);

    constexpr ConstArrayView cc;
    constexpr bool boolCc = !!cc;
    CORRADE_VERIFY(!boolCc);

    /** @todo constexpr void but not const? c++14? */

    constexpr ConstVoidArrayView cvb = Array30;
    constexpr bool boolCvb = !!cvb;
    CORRADE_VERIFY(boolCvb);

    constexpr ConstVoidArrayView cvc;
    constexpr bool boolCvc = !!cvc;
    CORRADE_VERIFY(!boolCvc);

    /* Explicit conversion to bool is allowed, but not to int */
    CORRADE_VERIFY(std::is_constructible<bool, ArrayView>::value);
    CORRADE_VERIFY(std::is_constructible<bool, VoidArrayView>::value);
    CORRADE_VERIFY(std::is_constructible<bool, ConstVoidArrayView>::value);
    CORRADE_VERIFY(!std::is_constructible<int, ArrayView>::value);
    CORRADE_VERIFY(!std::is_constructible<int, VoidArrayView>::value);
    CORRADE_VERIFY(!std::is_constructible<int, ConstVoidArrayView>::value);
}

void ArrayViewTest::convertPointer() {
    int a[7];
    ArrayView b = a;
    int* bp = b;
    CORRADE_COMPARE(bp, static_cast<int*>(a));

    const ArrayView c = a;
    const int* cp = c;
    CORRADE_COMPARE(cp, static_cast<const int*>(a));

    constexpr ConstArrayView cc = Array13;
    constexpr const int* ccp = cc;
    CORRADE_COMPARE(ccp, static_cast<const int*>(Array13));

    const ConstVoidArrayView d = a;
    const void* dp = d;
    CORRADE_COMPARE(dp, static_cast<const void*>(a));

    constexpr ConstVoidArrayView cd = Array30;
    constexpr const void* cdp = cd;
    CORRADE_COMPARE(cdp, static_cast<const void*>(Array30));

    /* Pointer arithmetic */
    const ArrayView e = a;
    const int* ep = e + 2;
    CORRADE_COMPARE(ep, &e[2]);
}

void ArrayViewTest::convertConst() {
    int a[3];
    ArrayView b = a;
    ConstArrayView c = b;
    CORRADE_VERIFY(c == a);
    CORRADE_COMPARE(c.size(), 3);
}

void ArrayViewTest::convertExternalView() {
    const int data[]{1, 2, 3, 4, 5};
    ConstIntView a{data, 5};
    CORRADE_COMPARE(a.data, data);
    CORRADE_COMPARE(a.size, 5);

    ConstArrayView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);

    ConstIntView c = b;
    CORRADE_COMPARE(c.data, data);
    CORRADE_COMPARE(c.size, 5);

    auto d = arrayView(c);
    CORRADE_VERIFY(std::is_same<decltype(d), Containers::ArrayView<const int>>::value);
    CORRADE_COMPARE(d.data(), data);
    CORRADE_COMPARE(d.size(), 5);

    constexpr ConstIntView ca{Array13, 13};
    CORRADE_COMPARE(ca.data, Array13);
    CORRADE_COMPARE(ca.size, 13);

    constexpr ConstArrayView cb = ca;
    CORRADE_COMPARE(cb.data(), Array13);
    CORRADE_COMPARE(cb.size(), 13);

    constexpr ConstIntView cc = cb;
    CORRADE_COMPARE(cc.data, Array13);
    CORRADE_COMPARE(cc.size, 13);

    constexpr auto cd = arrayView(cc);
    CORRADE_VERIFY(std::is_same<decltype(cd), const Containers::ArrayView<const int>>::value);
    CORRADE_COMPARE(cd.data(), Array13);
    CORRADE_COMPARE(cd.size(), 13);

    /* Conversion from/to a different type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Containers::ArrayView<const int>, ConstIntView>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::ArrayView<const float>, ConstIntView>::value);
    CORRADE_VERIFY(std::is_constructible<ConstIntView, Containers::ArrayView<const int>>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstIntView, Containers::ArrayView<const float>>::value);
}

void ArrayViewTest::convertConstFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstArrayView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5);

    /* Conversion from a different type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Containers::ArrayView<const int>, IntView>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::ArrayView<const float>, IntView>::value);

    /* Creating a non-const view from a const type should not be possible. Not
       using is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<Containers::ArrayView<const int>, ConstIntView>::value);
    CORRADE_VERIFY(!std::is_constructible<Containers::ArrayView<int>, ConstIntView>::value);
}

void ArrayViewTest::convertToConstExternalView() {
    int data[]{1, 2, 3, 4, 5};
    ArrayView a = data;
    CORRADE_COMPARE(a.data(), &data[0]);
    CORRADE_COMPARE(a.size(), 5);

    ConstIntView b = a;
    CORRADE_COMPARE(b.data, data);
    CORRADE_COMPARE(b.size, 5);

    /* Conversion to a different type is not allowed. Not using
       is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<ConstIntView, Containers::ArrayView<int>>::value);
    CORRADE_VERIFY(!std::is_constructible<ConstIntView, Containers::ArrayView<float>>::value);
}

void ArrayViewTest::convertVoidFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    VoidArrayView b = a;
    CORRADE_COMPARE(b.data(), &data[0]);
    CORRADE_COMPARE(b.size(), 5*4);
}

void ArrayViewTest::convertConstVoidFromExternalView() {
    int data[]{1, 2, 3, 4, 5};
    IntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstVoidArrayView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5*4);

    /* Creating a non-const view from a const type should not be possible. Not
       using is_convertible to catch also accidental explicit conversions. */
    CORRADE_VERIFY(std::is_constructible<ConstVoidArrayView, ConstIntView>::value);
    CORRADE_VERIFY(!std::is_constructible<VoidArrayView, ConstIntView>::value);
}

void ArrayViewTest::convertConstVoidFromConstExternalView() {
    const int data[]{1, 2, 3, 4, 5};
    ConstIntView a{data, 5};
    CORRADE_COMPARE(a.data, &data[0]);
    CORRADE_COMPARE(a.size, 5);

    ConstVoidArrayView b = a;
    CORRADE_COMPARE(b.data(), data);
    CORRADE_COMPARE(b.size(), 5*4);
}

/* Needs to be here in order to use it in constexpr */
constexpr int OneToSeven[]{0, 1, 2, 3, 4, 5, 6};

void ArrayViewTest::access() {
    int a[7];
    ArrayView b = a;
    for(std::size_t i = 0; i != 7; ++i)
        b[i] = i;

    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 7);
    CORRADE_COMPARE(b.front(), 0);
    CORRADE_COMPARE(b.back(), 6);
    CORRADE_COMPARE(*(b.begin()+2), 2);
    CORRADE_COMPARE(b[4], 4);
    CORRADE_COMPARE(b.end()-b.begin(), b.size());
    CORRADE_COMPARE(b.cbegin(), b.begin());
    CORRADE_COMPARE(b.cend(), b.end());

    Containers::ArrayView<const int> c = a;
    CORRADE_COMPARE(c.data(), a);

    constexpr ConstArrayView cb = OneToSeven;

    constexpr const int* data = cb.data();
    {
        /* FFS, there's still no common way to detect whether a sanitizer is
           enabled?! Former is Clang only, latter is GCC-only. */
        #ifdef __has_feature
        #if __has_feature(address_sanitizer)
        #define _CORRADE_ASAN_ENABLED
        #endif
        #elif defined(__SANITIZE_ADDRESS__)
        #define _CORRADE_ASAN_ENABLED
        #endif
        /* Fixed on Clang 15 again. Apple Clang 14 is the same as Clang 14, no
           need to special-case that (unbelievable!). */
        #if defined(CORRADE_TARGET_CLANG) && defined(_CORRADE_ASAN_ENABLED) && __clang_major__ == 14
        CORRADE_EXPECT_FAIL("Clang 14 with AddressSanitizer enabled seems to make a copy of the referenced array in this case, but not in case of begin() and end() below.");
        #endif
        CORRADE_VERIFY(data == OneToSeven);
    }

    constexpr std::size_t size = cb.size();
    CORRADE_COMPARE(size, 7);

    constexpr const int& front = cb.front();
    CORRADE_COMPARE(front, 0);

    constexpr const int& back = cb.back();
    CORRADE_COMPARE(back, 6);

    constexpr const int* begin = cb.begin();
    constexpr const int* cbegin = cb.cbegin();
    CORRADE_COMPARE(begin, OneToSeven);
    CORRADE_COMPARE(cbegin, OneToSeven);

    constexpr const int* end = cb.end();
    constexpr const int* cend = cb.cend();
    CORRADE_COMPARE(end, OneToSeven + 7);
    CORRADE_COMPARE(cend, OneToSeven + 7);

    constexpr int four = cb[4];
    CORRADE_COMPARE(four, 4);
}

void ArrayViewTest::accessConst() {
    /* The array is non-owning, so it should provide write access to the data */

    int a[7];
    const ArrayView b = a;
    b.front() = 0;
    *(b.begin()+1) = 1;
    *(b.cbegin()+2) = 2;
    b[3] = 3;
    *(b.end()-3) = 4;
    *(b.cend()-2) = 5;
    b.back() = 6;

    CORRADE_COMPARE(a[0], 0);
    CORRADE_COMPARE(a[1], 1);
    CORRADE_COMPARE(a[2], 2);
    CORRADE_COMPARE(a[3], 3);
    CORRADE_COMPARE(a[4], 4);
    CORRADE_COMPARE(a[5], 5);
    CORRADE_COMPARE(a[6], 6);
}

void ArrayViewTest::accessVoid() {
    int a[7]{};

    VoidArrayView b = a;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 7*sizeof(int));

    /** @todo constexpr but not const? c++14? */
}

void ArrayViewTest::accessConstVoid() {
    int a[7]{};

    ConstVoidArrayView b = a;
    CORRADE_VERIFY(b.data() == a);
    CORRADE_COMPARE(b.size(), 7*sizeof(int));

    constexpr ConstVoidArrayView cb = OneToSeven;

    constexpr const void* data = cb.data();
    CORRADE_VERIFY(data == OneToSeven);

    constexpr std::size_t size = cb.size();
    CORRADE_COMPARE(size, 7*sizeof(int));
}

void ArrayViewTest::accessInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Containers::String out;
    Error redirectError{&out};

    ArrayView a;
    a.front();
    a.back();
    CORRADE_COMPARE(out,
        "Containers::ArrayView::front(): view is empty\n"
        "Containers::ArrayView::back(): view is empty\n");
}

void ArrayViewTest::rangeBasedFor() {
    int a[5];
    ArrayView b = a;
    for(auto& i: b)
        i = 3;

    CORRADE_COMPARE(b[0], 3);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 3);
    CORRADE_COMPARE(b[3], 3);
    CORRADE_COMPARE(b[4], 3);
}

void ArrayViewTest::sliceInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    /* Do it this way to avoid (reasonable) warnings about out-of-bounds array
       access with `a - 1` */
    int data[6] = {0, 1, 2, 3, 4, 5};
    ArrayView a{data + 1, 5};

    CORRADE_COMPARE(a.size(), 5);

    Containers::String out;
    Error redirectError{&out};

    /* Testing both pointer and size versions */
    a.slice(a - 1, a);
    a.slice(a + 5, a + 6);
    a.slice(5, 6);
    a.slice(a + 2, a + 1);
    a.slice(2, 1);
    /* Testing template size + pointer, template size + size and full template
       version */
    a.slice<1>(a - 1);
    a.slice<5>(a + 1);
    a.slice<5>(1);
    a.slice<1, 6>();

    CORRADE_COMPARE(out,
        "Containers::ArrayView::slice(): slice [-1:0] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [5:6] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [2:1] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [2:1] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [-1:0] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [1:6] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [1:6] out of range for 5 elements\n"
        "Containers::ArrayView::slice(): slice [1:6] out of range for 5 elements\n");
}

void ArrayViewTest::sliceNullptr() {
    ArrayView a{nullptr, 5};

    ArrayView b = a.prefix(nullptr);
    CORRADE_VERIFY(!b);
    CORRADE_COMPARE(b.size(), 0);

    ArrayView c = a.suffix(nullptr);
    CORRADE_VERIFY(!c);
    CORRADE_COMPARE(c.size(), 5);

    constexpr ArrayView ca{nullptr, 5};

    constexpr ArrayView cb = ca.prefix(nullptr);
    CORRADE_VERIFY(!cb);
    CORRADE_COMPARE(cb.size(), 0);

    /* constexpr ArrayView cc = ca.suffix(nullptr) won't compile because
       arithmetic on nullptr is not allowed */

    int data[5];
    ArrayView d{data};

    ArrayView e = d.prefix(nullptr);
    CORRADE_VERIFY(!e);
    CORRADE_COMPARE(e.size(), 0);

    ArrayView f = d.suffix(nullptr);
    CORRADE_VERIFY(!f);
    CORRADE_COMPARE(f.size(), 0);

    constexpr ConstArrayView cd = Array13;
    constexpr ConstArrayView ce = cd.prefix(nullptr);
    CORRADE_VERIFY(!ce);
    CORRADE_COMPARE(ce.size(), 0);

    constexpr ConstArrayView cf = cd.suffix(nullptr);
    CORRADE_VERIFY(!cf);
    CORRADE_COMPARE(cf.size(), 0);
}

constexpr int Array5[]{1, 2, 3, 4, 5};

void ArrayViewTest::slice() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    ArrayView b1 = a.slice(1, 4);
    CORRADE_COMPARE(b1.size(), 3);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    ArrayView b2 = a.sliceSize(1, 3);
    CORRADE_COMPARE(b2.size(), 3);
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    ArrayView c = a.prefix(3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ArrayView d = a.exceptPrefix(2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    ArrayView e = a.exceptSuffix(2);
    CORRADE_COMPARE(e.size(), 3);
    CORRADE_COMPARE(e[0], 1);
    CORRADE_COMPARE(e[1], 2);
    CORRADE_COMPARE(e[2], 3);

    constexpr ConstArrayView ca = Array5;
    constexpr ConstArrayView cb = ca.slice(1, 4);
    CORRADE_COMPARE(cb.size(), 3);
    CORRADE_COMPARE(cb[0], 2);
    CORRADE_COMPARE(cb[1], 3);
    CORRADE_COMPARE(cb[2], 4);

    constexpr ConstArrayView cc = ca.prefix(3);
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    constexpr ConstArrayView cd = ca.exceptPrefix(2);
    CORRADE_COMPARE(cd.size(), 3);
    CORRADE_COMPARE(cd[0], 3);
    CORRADE_COMPARE(cd[1], 4);
    CORRADE_COMPARE(cd[2], 5);

    constexpr ConstArrayView ce = ca.exceptSuffix(2);
    CORRADE_COMPARE(ce.size(), 3);
    CORRADE_COMPARE(ce[0], 1);
    CORRADE_COMPARE(ce[1], 2);
    CORRADE_COMPARE(ce[2], 3);
}

void ArrayViewTest::slicePointer() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    ArrayView b1 = a.slice(data + 1, data + 4);
    CORRADE_COMPARE(b1.size(), 3);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    ArrayView b2 = a.sliceSize(data + 1, 3);
    CORRADE_COMPARE(b2.size(), 3);
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    ArrayView c = a.prefix(data + 3);
    CORRADE_COMPARE(c.size(), 3);
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    ArrayView d = a.suffix(data + 2);
    CORRADE_COMPARE(d.size(), 3);
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    /* MSVC 2015 chokes on all these due to (I assume) the assertion doing
       pointer arithmetic on the _data member. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr ConstArrayView ca = Array5;
    constexpr ConstArrayView cb = ca.slice(Array5 + 1, Array5 + 4);
    CORRADE_COMPARE(cb.size(), 3);
    CORRADE_COMPARE(cb[0], 2);
    CORRADE_COMPARE(cb[1], 3);
    CORRADE_COMPARE(cb[2], 4);

    /* The slice function checks for validity of the pointers, taking one
       pointer from _data and the second pointer from the prefix() argument.
       GCC <= 5 chokes on that, because for it doing pointer arithmetic on
       _data is apparently not constexpr. Note that the above slice() call
       worked correctly on it (both pointers treated as constexpr?). */
    #if !defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_CLANG) || __GNUC__ > 5
    constexpr ConstArrayView cc = ca.prefix(Array5 + 3);
    CORRADE_COMPARE(cc.size(), 3);
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    constexpr ConstArrayView cd = ca.suffix(Array5 + 2);
    CORRADE_COMPARE(cd.size(), 3);
    CORRADE_COMPARE(cd[0], 3);
    CORRADE_COMPARE(cd[1], 4);
    CORRADE_COMPARE(cd[2], 5);
    #endif
    #endif
}

void ArrayViewTest::sliceToStatic() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    StaticArrayView<3, int> b1 = a.slice<3>(1);
    CORRADE_COMPARE(b1[0], 2);
    CORRADE_COMPARE(b1[1], 3);
    CORRADE_COMPARE(b1[2], 4);

    StaticArrayView<3, int> b2 = a.slice<1, 4>();
    CORRADE_COMPARE(b2[0], 2);
    CORRADE_COMPARE(b2[1], 3);
    CORRADE_COMPARE(b2[2], 4);

    StaticArrayView<3, int> b3 = a.sliceSize<1, 3>();
    CORRADE_COMPARE(b3[0], 2);
    CORRADE_COMPARE(b3[1], 3);
    CORRADE_COMPARE(b3[2], 4);

    StaticArrayView<3, int> c = a.prefix<3>();
    CORRADE_COMPARE(c[0], 1);
    CORRADE_COMPARE(c[1], 2);
    CORRADE_COMPARE(c[2], 3);

    StaticArrayView<3, int> d = a.suffix<3>();
    CORRADE_COMPARE(d[0], 3);
    CORRADE_COMPARE(d[1], 4);
    CORRADE_COMPARE(d[2], 5);

    /* Similarly to above, MSVC 2015 chokes on this due to (I assume) doing
       pointer arithmetic on _data inside the assert. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr ConstArrayView ca = Array5;
    constexpr StaticArrayView<3, const int> cb1 = ca.slice<3>(1);
    CORRADE_COMPARE(cb1[0], 2);
    CORRADE_COMPARE(cb1[1], 3);
    CORRADE_COMPARE(cb1[2], 4);

    constexpr StaticArrayView<3, const int> cb2 = ca.slice<1, 4>();
    CORRADE_COMPARE(cb2[0], 2);
    CORRADE_COMPARE(cb2[1], 3);
    CORRADE_COMPARE(cb2[2], 4);

    constexpr StaticArrayView<3, const int> cc = ca.prefix<3>();
    CORRADE_COMPARE(cc[0], 1);
    CORRADE_COMPARE(cc[1], 2);
    CORRADE_COMPARE(cc[2], 3);

    constexpr StaticArrayView<3, const int> cd = ca.suffix<3>();
    CORRADE_COMPARE(cd[0], 3);
    CORRADE_COMPARE(cd[1], 4);
    CORRADE_COMPARE(cd[2], 5);
    #endif
}

void ArrayViewTest::sliceToStaticPointer() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    StaticArrayView<3, int> b = a.slice<3>(a + 1);
    CORRADE_COMPARE(b[0], 2);
    CORRADE_COMPARE(b[1], 3);
    CORRADE_COMPARE(b[2], 4);

    /* Similarly to above, MSVC 2015 chokes on this due to (I assume) doing
       pointer arithmetic on _data inside the assert. */
    #ifndef CORRADE_MSVC2015_COMPATIBILITY
    constexpr ConstArrayView ca = Array5;
    constexpr StaticArrayView<3, const int> cb = ca.slice<3>(ca + 1);
    CORRADE_COMPARE(cb[0], 2);
    CORRADE_COMPARE(cb[1], 3);
    CORRADE_COMPARE(cb[2], 4);
    #endif
}

void ArrayViewTest::sliceZeroNullPointerAmbiguity() {
    int data[5] = {1, 2, 3, 4, 5};
    ArrayView a = data;

    /* These should all unambigously pick the std::size_t overloads, not the
       T* overloads */

    ArrayView b = a.sliceSize(0, 3);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(b[0], 1);
    CORRADE_COMPARE(b[1], 2);
    CORRADE_COMPARE(b[2], 3);

    ArrayView c = a.prefix(0);
    CORRADE_COMPARE(c.size(), 0);
    CORRADE_COMPARE(c.data(), static_cast<void*>(a.data()));

    /** @todo suffix(0), once the non-deprecated suffix(std::size_t size) is a
        thing */

    StaticArrayView<3, int> e = a.slice<3>(0);
    CORRADE_COMPARE(e[0], 1);
    CORRADE_COMPARE(e[1], 2);
    CORRADE_COMPARE(e[2], 3);

    constexpr ConstArrayView ca = Array5;
    constexpr ConstArrayView cb = ca.sliceSize(0, 3);
    CORRADE_COMPARE(cb.size(), 3);
    CORRADE_COMPARE(cb[0], 1);
    CORRADE_COMPARE(cb[1], 2);
    CORRADE_COMPARE(cb[2], 3);

    constexpr ConstArrayView cc = ca.prefix(0);
    CORRADE_COMPARE(cc.size(), 0);
    CORRADE_COMPARE(cc.data(), static_cast<const void*>(ca.data()));

    /** @todo suffix(0), once the non-deprecated suffix(std::size_t size) is a
        thing */

    constexpr StaticArrayView<3, const int> ce = ca.slice<3>(0);
    CORRADE_COMPARE(ce[0], 1);
    CORRADE_COMPARE(ce[1], 2);
    CORRADE_COMPARE(ce[2], 3);
}

void ArrayViewTest::cast() {
    std::uint32_t data[6]{};
    Containers::ArrayView<std::uint32_t> a = data;
    Containers::ArrayView<void> av = data;
    Containers::ArrayView<const void> cav = data;
    auto b = Containers::arrayCast<std::uint64_t>(a);
    auto bv = Containers::arrayCast<std::uint64_t>(av);
    auto cbv = Containers::arrayCast<const std::uint64_t>(cav);
    auto c = Containers::arrayCast<std::uint16_t>(a);
    auto cv = Containers::arrayCast<std::uint16_t>(av);
    auto ccv = Containers::arrayCast<const std::uint16_t>(cav);

    CORRADE_VERIFY(std::is_same<decltype(b), Containers::ArrayView<std::uint64_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(bv), Containers::ArrayView<std::uint64_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cbv), Containers::ArrayView<const std::uint64_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(c), Containers::ArrayView<std::uint16_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(cv), Containers::ArrayView<std::uint16_t>>::value);
    CORRADE_VERIFY(std::is_same<decltype(ccv), Containers::ArrayView<const std::uint16_t>>::value);
    CORRADE_COMPARE(static_cast<void*>(b.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<void*>(bv.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<const void*>(cbv.begin()), static_cast<const void*>(a.begin()));
    CORRADE_COMPARE(static_cast<void*>(c.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<void*>(cv.begin()), static_cast<void*>(a.begin()));
    CORRADE_COMPARE(static_cast<const void*>(ccv.begin()), static_cast<const void*>(a.begin()));
    CORRADE_COMPARE(a.size(), 6);
    CORRADE_COMPARE(av.size(), 6*4);
    CORRADE_COMPARE(cav.size(), 6*4);
    CORRADE_COMPARE(b.size(), 3);
    CORRADE_COMPARE(bv.size(), 3);
    CORRADE_COMPARE(cbv.size(), 3);
    CORRADE_COMPARE(c.size(), 12);
    CORRADE_COMPARE(cv.size(), 12);
    CORRADE_COMPARE(ccv.size(), 12);
}

void ArrayViewTest::castInvalid() {
    CORRADE_SKIP_IF_NO_ASSERT();

    char data[10]{};
    Containers::ArrayView<char> a = data;
    Containers::ArrayView<void> av = data;
    Containers::ArrayView<const void> cav = data;

    auto b = Containers::arrayCast<std::uint16_t>(a);
    auto bv = Containers::arrayCast<std::uint16_t>(av);
    auto cbv = Containers::arrayCast<const std::uint16_t>(cav);
    CORRADE_COMPARE(b.size(), 5);
    CORRADE_COMPARE(bv.size(), 5);
    CORRADE_COMPARE(cbv.size(), 5);

    {
        Containers::String out;
        Error redirectError{&out};
        Containers::arrayCast<std::uint32_t>(a);
        Containers::arrayCast<std::uint32_t>(av);
        Containers::arrayCast<const std::uint32_t>(cav);
        CORRADE_COMPARE(out,
            "Containers::arrayCast(): can't reinterpret 10 1-byte items into a 4-byte type\n"
            "Containers::arrayCast(): can't reinterpret 10 bytes into a 4-byte type\n"
            "Containers::arrayCast(): can't reinterpret 10 bytes into a 4-byte type\n");
    }
}

void ArrayViewTest::size() {
    int a[6]{};
    ArrayView b{a, 3};
    ConstVoidArrayView c{a};
    struct D {
        int e[7];
    } d;

    CORRADE_COMPARE(Containers::arraySize(a), 6);
    CORRADE_COMPARE(Containers::arraySize(b), 3);
    CORRADE_COMPARE(Containers::arraySize(c), 24);
    CORRADE_COMPARE(Containers::arraySize(d.e), 7);
    CORRADE_COMPARE(Containers::arraySize(&D::e), 7);

    constexpr ConstArrayView cb{Array13, 3};
    constexpr ConstVoidArrayView cc{Array13};
    constexpr D cd{};
    constexpr std::size_t sizeA = Containers::arraySize(Array13);
    constexpr std::size_t sizeB = Containers::arraySize(cb);
    constexpr std::size_t sizeC = Containers::arraySize(cc);
    constexpr std::size_t sizeD1 = Containers::arraySize(cd.e);
    constexpr std::size_t sizeD2 = Containers::arraySize(&D::e);
    CORRADE_COMPARE(sizeA, 13);
    CORRADE_COMPARE(sizeB, 3);
    CORRADE_COMPARE(sizeC, 52);
    CORRADE_COMPARE(sizeD1, 7);
    CORRADE_COMPARE(sizeD2, 7);
}

}}}}

CORRADE_TEST_MAIN(Corrade::Containers::Test::ArrayViewTest)
