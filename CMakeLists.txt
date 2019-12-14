cmake_minimum_required(VERSION 3.1)

project(stouza)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

# Search path for CMake include files.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FindOpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

find_package(MHD REQUIRED)

# Enable C++ 11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(xxd ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/xxd/xxd.c)

file(GLOB_RECURSE SRCS "src/*.cpp")

include(AddFile)

file(GLOB_RECURSE WWW_SRCS "www/*")

# Embed WWW sources.
foreach(FILE ${WWW_SRCS})
	add_file(${FILE} SRCS)
endforeach()

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_executable(${PROJECT_NAME} ${SRCS})
target_compile_definitions(${PROJECT_NAME} PUBLIC GIT_SHA1="${GIT_SHA1}")
target_link_libraries(${PROJECT_NAME} ${Thread_LIBRARIES} ${MHD_LIBRARIES})

