########### Dragons Light INDI driver ##############
project(indi-dragonlight C CXX)
cmake_minimum_required(VERSION 3.0)

include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

find_package(INDI 1.9 REQUIRED)
find_package(CURL REQUIRED)

if (CMAKE_VERSION VERSION_LESS 3.12.0)
set(CURL ${CURL_LIBRARIES})
else()
set(CURL CURL::libcurl)
endif()

set(VERSION_MAJOR 1)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)

set(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/indi_dragonlight.xml.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/indi_dragonlight.xml
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${INDI_INCLUDE_DIR} )
include_directories( ${CURL_INCLUDE_DIR})

include(CMakeCommon)

add_executable(
    indi_dragonlight
    dragonlight.cpp
)

target_link_libraries(
    indi_dragonlight
    ${INDI_LIBRARIES}
    ${CURL}
)

install(TARGETS indi_dragonlight RUNTIME DESTINATION bin)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/indi_dragonlight.xml
    DESTINATION ${INDI_DATA_DIR}
)

execute_process(
    COMMAND bash "-c" "lsb_release --codename | cut -f2" 
    OUTPUT_VARIABLE DISTRO OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Rick Bassham")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libindi1 (>= 1.9)")
set(CPACK_DEBIAN_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${DISTRO}.deb")
include(CPack)
