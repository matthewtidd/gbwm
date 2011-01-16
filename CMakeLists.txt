cmake_minimum_required (VERSION 2.6)
project (gbwm)

set (PROJECT_SOURCE_DIR src)

set (GBWM_VERSION_MAJOR 0)
set (GBWM_VERSION_MINOR 3)
set (GBWM_VERSION_REV   0)

set (CMAKE_CXX_FLAGS "-g -Wall")

configure_file (
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
)

# the rest of the source needs to be able to see the config.h file
include_directories("${PROJECT_BINARY_DIR}")

find_package (PkgConfig)
pkg_check_modules (PANGO REQUIRED pangocairo)
pkg_check_modules (CAIRO REQUIRED cairo)

add_executable (gbwm src/gbwm.cpp src/screen.cpp src/client.cpp src/event.cpp src/window.cpp src/button.cpp src/titlebar.cpp)

link_directories (${PANGO_LIBRARY_DIRS} ${CAIRO_LIBRARY_DIRS})
include_directories (${PANGO_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS})
target_link_libraries (gbwm xcb xcb-icccm ${PANGO_LIBRARIES} ${CAIRO_LIBRARIES})
