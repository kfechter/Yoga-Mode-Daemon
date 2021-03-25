cmake_minimum_required(VERSION 3.1.0)
set(CMAKE_INSTALL_PREFIX "/")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")

project(thinkpad_yoga_mode_detection)

set (PROJECT_VERSION "1.0")
set (PACKAGE "${PROJECT_NAME}")
set (VERSION "${PROJECT_VERSION}")
set (PACKAGE_NAME "${PACKAGE}")
set (PACKAGE_TARNAME "${PACKAGE}")
set (PACKAGE_VERSION "${VERSION}")
set (PACKAGE_STRING "${PACKAGE} ${VERSION}")

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
link_libraries(${X11_Xinput_LIB})
include_directories(${X11_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "-O2")
set(CMAKE_CXX_FLAGS "-Wall")

set(SOURCE_FILES yoga-mode-switch.cpp ec-access.h)
add_executable(yoga-mode-switch ${SOURCE_FILES})
install(TARGETS yoga-mode-switch DESTINATION /usr/bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
            GROUP_EXECUTE GROUP_READ
            WORLD_EXECUTE WORLD_READ)