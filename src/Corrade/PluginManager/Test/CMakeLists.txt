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
set(CMAKE_FOLDER "Corrade/PluginManager/Test")

if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    # On windows, all DLLs are in bin/, but elsewhere in lib/. See the root
    # CMakeLists.txt for more info about where this is set to.
    if(CORRADE_TARGET_WINDOWS)
        set(PLUGINS_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        set(PLUGINS_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    endif()
    set(DOG_PLUGIN_FILENAME $<TARGET_FILE:Dog>)
    set(DOGGO_PLUGIN_FILENAME $<TARGET_FILE:Doggo>)
    set(PITBULL_PLUGIN_FILENAME $<TARGET_FILE:PitBull>)
endif()

# First replace ${} variables, then $<> generator expressions
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/configure.h
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/configure.h.in)

set(PluginManagerManagerTest_SRCS
    ManagerTest.cpp
    AbstractAnimal.cpp
    AbstractCustomSuffix.cpp
    AbstractDeletable.cpp
    AbstractDisabledMetadata.cpp
    AbstractFood.cpp)
if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    list(APPEND PluginManagerManagerTest_SRCS AbstractWrongPlugin.cpp)
endif()
corrade_add_test(PluginManagerManagerTest ${PluginManagerManagerTest_SRCS}
    LIBRARIES
        CorradePluginManagerTestLib
        Canary CustomSuffixStatic DisabledMetadataStatic
        GeneratedMetadataStatic)
target_include_directories(PluginManagerManagerTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    # So the plugins get properly built when building the test
    add_dependencies(PluginManagerManagerTest
        Bulldog Dog PitBull Snail
        CustomSuffix Deletable DisabledMetadata GeneratedMetadataDynamic
        Doggo
        HotDog HotDogWithSnail OldBread RottenTomato
        MissingLibraryDependency NoPluginFinalizer NoPluginInitializer
        NoPluginInstancer NoPluginInterface NoPluginVersion WrongMetadata)
endif()

corrade_add_test(PluginManagerManagerInitFiniTest
    ManagerInitFiniTest.cpp
    LIBRARIES InitFiniStatic CorradePluginManager)
if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    target_include_directories(PluginManagerManagerInitFiniTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    # So the plugins get properly built when building the test
    add_dependencies(PluginManagerManagerInitFiniTest InitFiniDynamic)
endif()

corrade_add_test(PluginManagerImportStaticTest
    AbstractAnimal.cpp
    ImportStaticTest.cpp
    LIBRARIES CorradePluginManagerTestLib Canary Dird)
target_include_directories(PluginManagerImportStaticTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

corrade_add_test(PluginManagerAbstractPluginTest
    AbstractAnimal.cpp
    AbstractPluginTest.cpp
    LIBRARIES CorradePluginManagerTestLib Canary)
target_include_directories(PluginManagerAbstractPluginTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

if(CORRADE_BUILD_STATIC)
    # CMake < 3.4 does this implicitly, but 3.4+ not anymore (see CMP0065).
    # That's generally okay, *except if* the build is static, the executable
    # uses a plugin manager and needs to share globals with the plugins (such
    # as output redirection and so on).
    set_target_properties(
        PluginManagerManagerTest
        PluginManagerManagerInitFiniTest
        PluginManagerImportStaticTest
        PluginManagerAbstractPluginTest
        PROPERTIES ENABLE_EXPORTS ON)
endif()

if(CORRADE_BUILD_STATIC AND NOT CORRADE_TARGET_EMSCRIPTEN AND NOT CORRADE_TARGET_IOS AND NOT CORRADE_TARGET_ANDROID AND NOT CORRADE_TARGET_WINDOWS_RT)
    # Yes, adding AbstractAnimal.cpp to both targets is fine in this case.
    # The whole Corrade gets duplicated too. Using CorradePluginManagerTestLib
    # instead of CorradePluginManager as that's what Canary links to.
    add_library(PluginManagerGlobalStateAcrossLibrariesLibrary SHARED
        AbstractAnimal.cpp
        GlobalStateAcrossLibrariesLibrary.cpp)
    target_link_libraries(PluginManagerGlobalStateAcrossLibrariesLibrary
        PUBLIC CorradePluginManagerTestLib
        PRIVATE Canary)
    target_include_directories(PluginManagerGlobalStateAcrossLibrariesLibrary PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

    corrade_add_test(PluginManagerGlobalStateAcrossLi___Test
        AbstractAnimal.cpp
        GlobalStateAcrossLibrariesTest.cpp
        LIBRARIES PluginManagerGlobalStateAcrossLibrariesLibrary)
    target_include_directories(PluginManagerGlobalStateAcrossLi___Test PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

    add_library(DuplicateStaticPluginLibrary1 SHARED
        AbstractAnimal.cpp
        DuplicateStaticPluginLibrary.cpp)
    add_library(DuplicateStaticPluginLibrary2 SHARED
        AbstractAnimal.cpp
        DuplicateStaticPluginLibrary.cpp)
    target_link_libraries(DuplicateStaticPluginLibrary1
        PUBLIC CorradePluginManagerTestLib
        PRIVATE Canary)
    target_link_libraries(DuplicateStaticPluginLibrary2
        PUBLIC CorradePluginManagerTestLib
        PRIVATE Canary Dird)
    target_compile_definitions(DuplicateStaticPluginLibrary1 PRIVATE BUILDING_LIBRARY1)
    target_compile_definitions(DuplicateStaticPluginLibrary2 PRIVATE BUILDING_LIBRARY2)
    target_include_directories(DuplicateStaticPluginLibrary1 PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    target_include_directories(DuplicateStaticPluginLibrary2 PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)

    corrade_add_test(PluginManagerDuplicateStaticPluginTest
        AbstractAnimal.cpp
        DuplicateStaticPluginTest.cpp
        LIBRARIES DuplicateStaticPluginLibrary1 DuplicateStaticPluginLibrary2)
    target_include_directories(PluginManagerDuplicateStaticPluginTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
endif()

add_subdirectory(animals)
add_subdirectory(init-fini)
add_subdirectory(custom-suffix)
add_subdirectory(disabled-metadata)
add_subdirectory(generated-metadata)
if(NOT CORRADE_PLUGINMANAGER_NO_DYNAMIC_PLUGIN_SUPPORT)
    add_subdirectory(doggo)
    add_subdirectory(deletable)
    add_subdirectory(food)
    add_subdirectory(wrong)
    add_subdirectory(wrong-metadata)
endif()
