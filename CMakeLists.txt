cmake_minimum_required(VERSION 3.0.2)
project(lade C)
set(PROJECT_VERSION 0.01)
set(CMAKE_BUILD_TYPE Debug)

include(CheckIncludeFiles)
include(CheckTypeSize)
check_include_files(pid.h HAVE_PID_H)
check_include_files(sys/ptrace.h HAVE_PTRACE_H)
check_include_files(libproc.h HAVE_LIBPROC_H)
check_type_size(off_t OFF_T)
check_type_size(pid_t PID_T)

# TODO make this optional
find_package(PkgConfig OPTIONAL_COMPONENTS)
IF (PkgConfig_FOUND)
    pkg_search_module(LIBPID OPTIONAL_COMPONENTS libpid)
ENDIF()

IF(NOT LIBPID_FOUND)
    IF(EXISTS libpid.dll)
        set(LIBPID_FOUND 1)
        set(LIBPID_LIBRARIES "-L. libpid.dll")
    ENDIF()
ENDIF()
IF(LIBPID_FOUND)
    message(STATUS "Using libpid for process identification")
    set(HAVE_LIBPID 1)
ELSE()
    MESSAGE(FATAL_ERROR "Compiling without libpid. Will only work on pids directly")
ENDIF()

IF(HAVE_OFF_T)
    set(HAVE_OFF_T 1)
ENDIF()

IF(HAVE_PID_T)
    set(LADE_HAVE_PID_T 1)
ENDIF()
check_include_files(sys/mman.h HAVE_MMAN_H)
IF(HAVE_MMAN_H)
    include(CheckSymbolExists)
    check_symbol_exists(MAP_ANONYMOUS sys/mman.h HAVE_MAP_ANONYMOUS)
    IF(NOT HAVE_MAP_ANONYMOUS)
        check_symbol_exists(MAP_ANON sys/mman.h HAVE_MAP_ANON)
    ENDIF()
ENDIF(HAVE_MMAN_H)


configure_file(lade.h.in lade.h)
#configure_file(lade-types.h.in lade-types.h)
configure_file(config.h.in config.h)
configure_file(lade.pc.in lade.pc @ONLY)

include(${CMAKE_CURRENT_SOURCE_DIR}/CFLAGS.cmake)

include_directories(${CMAKE_CURRENT_BINARY_DIR} include .)

#file(GLOB SOURCES info.c)

string(REGEX MATCH "Linux" LINUX ${CMAKE_SYSTEM_NAME})
string(REGEX MATCH "BSD" BSD ${CMAKE_SYSTEM_NAME})

IF(WIN32)
    message(STATUS "Configuring ${PROJECT_NAME} for WinAPI")
    set(LADE_ON_WIN32 1)
    add_definitions(-DNO_OLDNAMES -DWIN32_LEAN_AND_MEAN)
    file(GLOB SOURCES ${SOURCES} win32/*.c)

ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message(STATUS "Configuring ${PROJECT_NAME} for XNU")
    set(LADE_ON_XNU 1)
    set(CMAKE_MACOSX_RPATH 1) 
    set(SOURCES ${SOURCES} xnu/lade.c xnu/mach_inject.c)
    set(BOOTSTRAP_SOURCES xnu/bootstrap.c)

ELSEIF(HAVE_PTRACE_H) # Linux and more
    message(FATAL_ERROR "This platform is not yet supported.")
    message(STATUS "Configuring ${PROJECT_NAME} for ptrace(2)")
    set(LADE_ON_PTRACE 1)
    file(GLOB SOURCES ${SOURCES} ptrace/*.c)
    #add_definitions(-DHAS_WEAK_ALIAS)
ELSE()
    message(FATAL_ERROR "This platform is not supported.")
ENDIF(WIN32)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
IF (DEFINED BOOTSTRAP_SOURCES)
    add_library(${PROJECT_NAME}-bootstrap SHARED ${BOOTSTRAP_SOURCES})
ENDIF()

file(GLOB ${PROJECT_NAME}_cli_src *.c)
add_executable(${PROJECT_NAME}-cli ${lade_cli_src})
# TODO: make this target specific
include_directories(SYSTEM ${LIBPID_INCLUDE_DIRS})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LIBPID_CFLAGS_OTHER}")
target_link_libraries(${PROJECT_NAME}-cli ${PROJECT_NAME} ${LIBPID_LIBRARIES})
set_target_properties(${PROJECT_NAME}-cli PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

# Install pkg-config pc
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION share/pkgconfig)

# Install library and executable
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-cli
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Optionally install bootstrap library
IF (DEFINED BOOTSTRAP_SOURCES)
    install(TARGETS ${PROJECT_NAME}-bootstrap LIBRARY DESTINATION lib)
ENDIF()

# Install library headers
install(FILES ${CMAKE_BINARY_DIR}/lade.h DESTINATION include)

