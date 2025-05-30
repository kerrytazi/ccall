cmake_minimum_required (VERSION 3.10)
project(ccall)

set(SOURCE_LIST
	include/ccall/ccall.hpp
)

if(WIN32)
	enable_language(ASM_MASM)
	set(SOURCE_LIST ${SOURCE_LIST} native/ccall-win-x64.asm)
endif()

if(UNIX)
	enable_language(ASM_NASM)
	set(SOURCE_LIST ${SOURCE_LIST} native/ccall-linux-x64.asm)
endif()

add_library(ccall ${SOURCE_LIST})

set_property(TARGET ccall PROPERTY CXX_STANDARD 20)

target_include_directories(ccall PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


# cmake -DCCALL_TEST=ON -B build
option(CCALL_TEST "Build Test project" OFF)

if(CCALL_TEST)
	message(STATUS "Building Test")
	add_executable(ccall-test tests/test.cpp)
	set_property(TARGET ccall-test PROPERTY CXX_STANDARD 20)
	target_link_libraries(ccall-test ccall)

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ccall-test)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

