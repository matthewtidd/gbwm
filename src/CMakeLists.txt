find_package (PkgConfig)
pkg_check_modules (PANGO REQUIRED pangocairo)
pkg_check_modules (CAIRO REQUIRED cairo)

add_executable (gbwm gbwm.cpp screen.cpp client.cpp event.cpp window.cpp button.cpp titlebar.cpp)

link_directories (${PANGO_LIBRARY_DIRS} ${CAIRO_LIBRARY_DIRS})
include_directories (${PANGO_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS})
target_link_libraries (gbwm xcb xcb-icccm ${PANGO_LIBRARIES} ${CAIRO_LIBRARIES})

