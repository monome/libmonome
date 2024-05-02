cmake_minimum_required(VERSION 3.25)

project(libmonome VERSION 1.4.7)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

set(CMAKE_C_STANDARD 17)

include(GNUInstallDirs)

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
if(NOT MSVC)
    target_compile_options(monome_static PRIVATE -Wall -Werror -Wno-initializer-overrides)
endif()

add_library(monome SHARED ${libmonome_sources})
target_compile_definitions(monome PRIVATE EMBED_PROTOS)
target_include_directories(monome PRIVATE src/private)
target_include_directories(monome PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/public>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(monome PRIVATE ${libmonome_libs})
if(NOT MSVC)
    target_compile_options(monome PRIVATE -Wall -Werror -Wno-initializer-overrides)
endif()

install(TARGETS monome_static monome EXPORT libmonomeConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT libmonomeConfig DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libmonome NAMESPACE libmonome::)
install(DIRECTORY public/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(LINUX AND EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  include(InstallRequiredSystemLibraries)

  set(CPACK_GENERATOR "DEB")

  set(CPACK_PACKAGE_DESCRIPTION 
"It was developed to make monome devices easy to use with programming
languages like C and Python, and adding wrappers for use in your favorite
language with a suitable FFI is straightforward.

libmonome has support for 40h and series devices through a unified API,
and by default includes a third backend which wraps around the OSC
protocol and presents the same API as a physical device. This means that a
program written using libmonome can, at runtime, decide whether to
communicate with a running monomeserial instance over OSC or whether to
access the physical device directly.
  ")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A library for easy interaction with monome devices.")
  set(CPACK_PACKAGE_VENDOR "monome")
  set(CPACK_PACKAGE_CONTACT "help@monome.org")

  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT")

  set(CPACK_DEBIAN_PACKAGE_RELEASE 1)
  set(CPACK_DEBIAN_PACKAGE_VERSION ${PROJECT_VERSION})
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/monome/libmonome")
  set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "liblo7 (>= 0.31), libudev1 (>= 252.17)")
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

  include(CPack)
endif()
