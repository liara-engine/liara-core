cmake_minimum_required(VERSION 3.29)

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
    message(FATAL_ERROR
            "Liara::Interfaces is not available. Build the core through the workspace superbuild "
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

file(GLOB_RECURSE LIARA_CORE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.inl"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.ipp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.tpp"
)

add_library(liara_core STATIC ${LIARA_CORE_SOURCES})
add_library(Liara::Core ALIAS liara_core)

target_link_libraries(liara_core PRIVATE Liara::Interfaces)

target_include_directories(liara_core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(LIARA_CORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(LIARA_CORE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(DIRECTORY include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING
            PATTERN "*.h"
            PATTERN "config.h.in" EXCLUDE
    )

    install(
            FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/config.h"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/liara/core"
    )

    install(TARGETS liara_core
            EXPORT LiaraCoreTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT LiaraCoreTargets
            FILE LiaraCoreTargets.cmake
            NAMESPACE Liara::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LiaraCore
    )

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LiaraCoreConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraCoreConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LiaraCore
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraCoreConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraCoreConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/LiaraCoreConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LiaraCore
    )
endif()
