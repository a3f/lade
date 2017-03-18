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
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(${CMAKE_CURRENT_SOURCE_DIR}/CFLAGS.cmake)

include_directories(include)
include_directories(.)

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
    message(FATAL_ERROR "This platform is not supported yet.")
    message(STATUS "Configuring ${PROJECT_NAME} for ptrace(2)")
    set(LADE_ON_PTRACE 1)
    include(CheckFunctionExists)
        check_function_exists(pwrite HAVE_PWRITE )
        check_function_exists(pread HAVE_PREAD)
        IF (NOT HAVE_PWRITE OR NOT HAVE_PREAD)
            message(WARNING "using own implementation for pread/pwrite(2)")
        ENDIF()
    configure_file(config.h.in config.h)
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
target_link_libraries(${PROJECT_NAME}-cli ${PROJECT_NAME})
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

