
# CMakeLists.txt


cmake_minimum_required(VERSION 2.8)
project(libq) 

set(PACKAGE "libq")
set(PACKAGE_NAME "Quetzalcoatl Library")
set(PACKAGE_TITLE "${PACKAGE_NAME} General C Library")
set(PACKAGE_VERSION_MAJOR 0)
set(PACKAGE_VERSION_MINOR 1)
set(PACKAGE_VERSION_BUILD 0)
set(PACKAGE_MAINTAINER "Per Löwgren")
set(PACKAGE_BUGREPORT "per.lowgren@gmail.com")
set(PACKAGE_URL "")

set(USE_GTK2 2.14)

# Set the following options to ON to build support, or run CMake with flags -DUSE_[PARAM]=[ON/OFF]
option(USE_THREADS "Build with thread capabilities" ON)
option(USE_GETTEXT "Include GetText functionality" OFF)
option(USE_OPENSSL "Build with OpenSSL" ON)
option(USE_CURL "Build with cURL" ON)
option(USE_MYSQL "Use MySQL" ON)
option(USE_POSTGRESQL "Use PostgreSQL" OFF)
option(USE_SQLITE3 "Use SQLite 3" ON)

if(USE_MYSQL OR USE_POSTGRESQL OR USE_SQLITE3)
	set(USE_DB 1)
endif()

include(libq.cmake)

file(GLOB libq_src
	libq/*.c
	libq/glib/*.c
	libq/gtk/*.c
	libq/astro/*.c
	libq/astro/planets/*.c
)

add_library(q STATIC ${libq_src})
target_link_libraries(q ${LIBQ_LIBRARIES})
set_property(TARGET q PROPERTY C_STANDARD 11)

install(TARGETS q DESTINATION "${PACKAGE_LIBRARY_DIR}")

file(GLOB libq_h RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "libq/*.h")
list(FILTER libq_h EXCLUDE REGEX "^libq\\/_.+") 
install(FILES
	${libq_h}
	${PROJECT_BINARY_DIR}/src/libq/config.h
	DESTINATION include/libq
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/examples)

add_executable(lzw examples/lzw.c)
target_link_libraries(lzw q ${LIBQ_LIBRARIES})

add_executable(r20 examples/r20.c)
target_link_libraries(r20 q ${LIBQ_LIBRARIES})

add_executable(horoscope examples/horoscope.c)
target_link_libraries(horoscope q m)



