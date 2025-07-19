cmake_minimum_required(VERSION 3.27)

project(libmonome VERSION 1.4.8)

set(CMAKE_C_STANDARD 17)

option(BUILD_EXAMPLES "build libmonome c examples" ON)
option(BUILD_PYTHON_EXTENSION "build cython-based python extension")

include(GNUInstallDirs)

if (MSVC)
    add_compile_options(/W4)
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Werror)
elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Werror -Wno-initializer-overrides)
endif()

set(libmonome_sources
    src/libmonome.c
    src/monobright.c
    src/rotation.c
    src/proto/40h.c
    src/proto/mext.c
    src/proto/series.c
    src/platform/embed.c)

set(libmonome_libs)

if(LINUX)
    include(FindPkgConfig)
    pkg_check_modules(libudev REQUIRED IMPORTED_TARGET libudev)

    list(APPEND libmonome_sources
        src/platform/linux_libudev.c
        src/platform/linux.c
        src/platform/posix.c)
    list(APPEND libmonome_libs PkgConfig::libudev)
endif()

if(APPLE)
    list(APPEND libmonome_sources
        src/platform/darwin.c
        src/platform/posix.c)
endif()

if(WIN32)
    list(APPEND libmonome_sources src/platform/windows.c)
    list(APPEND libmonome_libs setupapi)
endif()

add_library(monome_static STATIC ${libmonome_sources})
set_target_properties(monome_static PROPERTIES OUTPUT_NAME "monome")
target_compile_definitions(monome_static PRIVATE EMBED_PROTOS)
target_include_directories(monome_static PRIVATE src/private)
target_include_directories(monome_static PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/public>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(monome_static PRIVATE ${libmonome_libs})

add_library(monome SHARED ${libmonome_sources})
target_compile_definitions(monome PRIVATE EMBED_PROTOS)
target_include_directories(monome PRIVATE src/private)
target_include_directories(monome PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/public>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(monome PRIVATE ${libmonome_libs})

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_PYTHON_EXTENSION)
    add_subdirectory(bindings/python)
endif()

install(TARGETS monome_static monome EXPORT libmonomeConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT libmonomeConfig DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmonome NAMESPACE libmonome::)
install(DIRECTORY public/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
