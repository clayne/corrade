/* Compiled resource file. DO NOT EDIT! */

#include "Corrade/Corrade.h"
#include "Corrade/Utility/Macros.h"
#include "Corrade/Utility/Resource.h"

#if CORRADE_RESOURCE_VERSION != 2
#ifdef CORRADE_TARGET_CLANG
#pragma GCC error "resource file compiled in version 2 but version " _CORRADE_HELPER_STR2(CORRADE_RESOURCE_VERSION) " expected, update your corrade-rc binary"
#else
#error resource file compiled in unexpected version 2, update your corrade-rc binary
#if defined(CORRADE_TARGET_GCC) || defined(CORRADE_TARGET_MSVC)
#pragma message("resource file version " _CORRADE_HELPER_STR2(CORRADE_RESOURCE_VERSION) " expected instead")
#endif
#endif
#endif

namespace {

/* Pair `i` is offset of filename `i + 1` in the low 24 bits, padding after
   data `i` in the upper 8 bits, and a 32bit offset of data `i + 1`. Offset of
   the first filename and data is implicitly 0. */
const unsigned int resourcePositions[] = {
    0x0000000e,0x00000011,
    0x01000013,0x00000052,
    0x2e000025,0x00000080
};

const unsigned char resourceFilenames[] = {
    /* 0-align128.bin */
    0x30,0x2d,0x61,0x6c,0x69,0x67,0x6e,0x31,0x32,0x38,0x2e,0x62,0x69,0x6e,

    /* 1.bin */
    0x31,0x2e,0x62,0x69,0x6e,

    /* 2-align2-empty.bin */
    0x32,0x2d,0x61,0x6c,0x69,0x67,0x6e,0x32,0x2d,0x65,0x6d,0x70,0x74,0x79,0x2e,
    0x62,0x69,0x6e
};

alignas(128) const unsigned char resourceData[] = {
    /* 0-align128.bin */
    0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,
    0x66,0x66,

    /* 1.bin */
    0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
    0x33,0x33,0x33,0x33,   0,

    /* 2-align2-empty.bin */
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0
};

Corrade::Utility::Implementation::ResourceGroup resource;

}

int corradeResourceInitializer_ResourceTestAlignmentLargerThanDataSizeData();
int corradeResourceInitializer_ResourceTestAlignmentLargerThanDataSizeData() {
    resource.name = "alignmentLargerThanDataSize";
    resource.count = 3;
    resource.positions = resourcePositions;
    resource.filenames = resourceFilenames;
    resource.data = resourceData;
    Corrade::Utility::Resource::registerData(resource);
    return 1;
} CORRADE_AUTOMATIC_INITIALIZER(corradeResourceInitializer_ResourceTestAlignmentLargerThanDataSizeData)

int corradeResourceFinalizer_ResourceTestAlignmentLargerThanDataSizeData();
int corradeResourceFinalizer_ResourceTestAlignmentLargerThanDataSizeData() {
    Corrade::Utility::Resource::unregisterData(resource);
    return 1;
} CORRADE_AUTOMATIC_FINALIZER(corradeResourceFinalizer_ResourceTestAlignmentLargerThanDataSizeData)
