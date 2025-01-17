#ifndef Corrade_Utility_DebugStl_h
#define Corrade_Utility_DebugStl_h
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

/** @file
@brief STL compatibility for @ref Corrade::Utility::Debug
@m_since{2019,10}

Including this header allows you to use STL types such as @ref std::string or
@ref std::tuple with @ref Corrade::Utility::Debug, as well as a fallback to
@ref std::ostream @cpp operator<<() @ce overloads if a type doesn't provide
@ref Corrade::Utility::operator<<(Debug&, const T&) "operator<<(Utility::Debug&, const T&)"
directly. A separate @ref Corrade/Utility/DebugStlStringView.h header provides
compatibility with @ref std::string_view from C++17. See @ref Utility-Debug-stl
for more information.
*/

#include <iosfwd>
#include <string>
/* this one doesn't add much on top of <string>, so it doesn't need to be
   separate */
#include <tuple>
#include <utility> /* std::pair */

#include "Corrade/Containers/ArrayView.h"
#include "Corrade/Containers/sequenceHelpers.h"
#include "Corrade/Utility/Debug.h"

namespace Corrade { namespace Utility {

namespace Implementation {
    CORRADE_HAS_TYPE(HasOstreamOutput, decltype(std::declval<std::ostream&>() << std::declval<T>()));

    CORRADE_UTILITY_EXPORT Debug& debugPrintStlString(Debug& debug, const std::string& value);
}

/** @relatesalso Debug
@brief Print a @ref std::pair to debug output

Prints the value as @cb{.shell-session} (first, second) @ce. Unlike
@ref operator<<(Debug& debug, const Iterable& value), the output is not
affected by @ref Debug::Flag::Packed / @ref Debug::packed.
*/
template<class T, class U> Debug& operator<<(Debug& debug, const std::pair<T, U>& value) {
    /* Nested values should get printed with the same flags, so make all
       immediate flags temporarily global -- except NoSpace, unless it's also
       set globally */
    const Debug::Flags prevFlags = debug.flags();
    debug.setFlags(prevFlags | (debug.immediateFlags() & ~Debug::Flag::NoSpace));

    debug << "(" << Debug::nospace << value.first << Debug::nospace << "," << value.second << Debug::nospace << ")";

    /* Reset the original flags back */
    debug.setFlags(prevFlags);

    return debug;
}

/** @relatesalso Debug
@brief Print a @ref std::string to debug output

For types that are only convertible to a @ref std::string this overload is
picked only if the type doesn't also provide a @ref std::ostream
@cpp operator<<() @ce overload. In that case the value is printed directly to
the stream instead, assuming it's a cheaper operation than conversion to a
@ref std::string. This is for example a case with @ref std::filesystem::path.
*/
#ifdef DOXYGEN_GENERATING_OUTPUT
Debug& operator<<(Debug& debug, const std::string& value);
#else
template<class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, std::string>::value || (std::is_convertible<T, std::string>::value && !Implementation::HasOstreamOutput<T>::value), int>::type = 0> Debug& operator<<(Debug& debug, const T& value) {
    return Implementation::debugPrintStlString(debug, value);
}
#endif

/** @relatesalso Debug
@brief Print a @ref std::basic_string to debug output

All other types than exactly @ref std::string are printed as containers.
*/
template<class T
    #ifndef DOXYGEN_GENERATING_OUTPUT
    , typename std::enable_if<!std::is_same<T, char>::value, int>::type = 0
    #endif
> Debug& operator<<(Debug& debug, const std::basic_string<T>& value) {
    return debug << Containers::ArrayView<const T>{value.data(), value.size()};
}

namespace Implementation {
    /* Used by operator<<(Debug&, std::tuple<>...) */
    template<class T> inline void tupleDebugOutput(Debug&, const T&, Containers::Implementation::Sequence<>) {}
    template<class T, std::size_t i, std::size_t ...sequence> void tupleDebugOutput(Debug& debug, const T& tuple, Containers::Implementation::Sequence<i, sequence...>) {
        debug << std::get<i>(tuple);
        #ifdef CORRADE_TARGET_MSVC
        #pragma warning(push)
        #pragma warning(disable:4127) /* conditional expression is constant (of course) */
        #endif
        if(i + 1 != std::tuple_size<T>::value)
            debug << Debug::nospace << ",";
        #ifdef CORRADE_TARGET_MSVC
        #pragma warning(pop)
        #endif
        tupleDebugOutput(debug, tuple, Containers::Implementation::Sequence<sequence...>{});
    }
}

/** @relatesalso Debug
@brief Print a @ref std::tuple to debug output

Prints the value as @cb{.shell-session} (first, second, third...) @ce. Unlike
@ref operator<<(Debug& debug, const Iterable& value), the output is not
affected by @ref Debug::Flag::Packed / @ref Debug::packed.
*/
template<class ...Args> Debug& operator<<(Debug& debug, const std::tuple<Args...>& value) {
    /* Nested values should get printed with the same flags, so make all
       immediate flags temporarily global -- except NoSpace, unless it's also
       set globally */
    const Debug::Flags prevFlags = debug.flags();
    debug.setFlags(prevFlags | (debug.immediateFlags() & ~Debug::Flag::NoSpace));

    debug << "(" << Debug::nospace;
    Implementation::tupleDebugOutput(debug, value, typename Containers::Implementation::GenerateSequence<sizeof...(Args)>::Type{});
    debug << Debug::nospace << ")";

    /* Reset the original flags back */
    debug.setFlags(prevFlags);

    return debug;
}

namespace Implementation {

/* Used by Debug::operator<<(Implementation::DebugOstreamFallback&&) */
struct DebugOstreamFallback {
    /** @todo without an explicit operator<<() for MutableBitArrayView,
        printing it causes an ambiguous overload between this and BitArrayView
        debug operator, adding
            class = decltype(std::declval<std::ostream&>() << std::declval<const T&>())
        fixes it on GCC but causes an infinite SFINAE recursion on Clang */
    template<class T> /*implicit*/ DebugOstreamFallback(const T& t): applier(&DebugOstreamFallback::applyImpl<T>), value(&t) {}

    void apply(std::ostream& s) const {
        (this->*applier)(s);
    }

    template<class T> void applyImpl(std::ostream& s) const {
        s << *static_cast<const T*>(value);
    }

    using ApplierFunc = void(DebugOstreamFallback::*)(std::ostream&) const;
    const ApplierFunc applier;
    const void* value;
};

}

#ifndef DOXYGEN_GENERATING_OUTPUT
/* This is in order to support printing types that have ostream operator<<
   implemented */
CORRADE_UTILITY_EXPORT Debug& operator<<(Debug& debug, Implementation::DebugOstreamFallback&& value);
#endif

}}

#endif
