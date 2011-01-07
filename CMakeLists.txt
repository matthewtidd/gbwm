cmake_minimum_required (VERSION 2.6)
project (gbwm)

set (PROJECT_SOURCE_DIR src)

set (GBWM_VERSION_MAJOR 0)
set (GBWM_VERSION_MINOR 2)

configure_file (
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
)

# the rest of the source needs to be able to see the config.h file
include_directories("${PROJECT_BINARY_DIR}")

add_subdirectory(src)
