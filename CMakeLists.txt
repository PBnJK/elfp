# CMake build

cmake_minimum_required(VERSION 3.28)
project(
	elfp
	VERSION 0.1
	LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
	elfp
	"src/main.c"
	"src/elfdump.c"
	"src/elfp.c"
	"src/util.c"
)

target_include_directories(elfp PRIVATE ${PROJECT_SOURCE_DIR}/inc)

target_compile_options(elfp PRIVATE -std=c99 -Wall -Wextra -pedantic)
