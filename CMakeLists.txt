cmake_minimum_required(VERSION 3.29)

include(GNUInstallDirs)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.release-please-manifest.json" RELEASE_PLEASE_MANIFEST)
string(JSON PROJECT_VERSION GET ${RELEASE_PLEASE_MANIFEST} ".")
message(STATUS "LiaraCore version: ${PROJECT_VERSION}")

project(LiaraCore
        VERSION ${PROJECT_VERSION}
        DESCRIPTION "Liara Engine core"
        HOMEPAGE_URL "https://github.com/liara-engine/liara-core"
        LANGUAGES CXX
)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/include/liara/core/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/config.h"
        @ONLY)

if(NOT TARGET Liara::Interfaces)
    find_package(LiaraInterfaces CONFIG QUIET)
endif()

if(NOT TARGET Liara::Interfaces)
    message(FATAL_ERROR
            "Liara::Interfaces is not available. Either build liara-core through the workspace superbuild, "
            "or install liara-interfaces and point find_package(LiaraInterfaces) at it via CMAKE_PREFIX_PATH "
            " (see https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/book/BOOTSTRAP)")
endif()

if(PROJECT_IS_TOP_LEVEL)
    set(LIARA_CORE_IS_TOP_LEVEL ON)
else()
    set(LIARA_CORE_IS_TOP_LEVEL OFF)
endif()

option(LIARA_CORE_BUILD_TESTS
        "Build the liara-core test suite."
        ${LIARA_CORE_IS_TOP_LEVEL})
option(LIARA_CORE_INSTALL
        "Generate install targets for liara-core."
        ${LIARA_CORE_IS_TOP_LEVEL})

if(LIARA_CORE_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

file(GLOB_RECURSE LIARA_CORE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(liara_core ${LIARA_CORE_SOURCES})
add_library(Liara::Core ALIAS liara_core)

target_link_libraries(liara_core PUBLIC Liara::Interfaces)

target_include_directories(liara_core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(liara_core PRIVATE LIARA_CORE_BUILD)

set_target_properties(liara_core PROPERTIES
        EXPORT_NAME Core
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
        SOVERSION ${PROJECT_VERSION_MAJOR}
        VERSION ${PROJECT_VERSION}
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(liara_core PRIVATE LIARA_CORE_SHARED)
else()
    target_compile_definitions(liara_core PRIVATE LIARA_CORE_STATIC)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(liara_core PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden)
endif()

if(LIARA_CORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(LIARA_CORE_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS liara_core
            EXPORT LiaraCoreTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/liara/core"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/liara"
            FILES_MATCHING PATTERN "*.h"
    )
    install(EXPORT LiaraCoreTargets
            FILE LiaraCoreTargets.cmake
            NAMESPACE Liara::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LiaraCore
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraCoreConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LiaraCoreConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraCoreConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LiaraCore
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraCoreConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraCoreConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LiaraCore
    )
endif()
