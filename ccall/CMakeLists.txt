set(SOURCE_LIST
	main.cpp
)

if(WIN32)
	enable_language(ASM_MASM)
	set(SOURCE_LIST ${SOURCE_LIST} ccall-win-x64.asm)
endif()

if(UNIX)
	enable_language(ASM_NASM)
	set(SOURCE_LIST ${SOURCE_LIST} ccall-linux-x64.asm)
endif()

add_executable(ccall ${SOURCE_LIST})

set_property(TARGET ccall PROPERTY CXX_STANDARD 20)
