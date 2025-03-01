cmake_minimum_required(VERSION 3.17)
project(socklib LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(PLATFORM "windows")
    set(CMAKE_GENERATOR vs2022)
elseif (APPLE)
    set(PLATFORM "apple")
elseif (UNIX AND NOT APPLE)
    set(PLATFORM "linux")
    set(CMAKE_GENERATOR Ninja Multi-Config)
else ()
    message("Not supported platform")
endif ()

# Output directories
set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin/$<CONFIG>-${PLATFORM}/socklib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin/$<CONFIG>-${PLATFORM}/socklib)

# Include directories
set(CATCH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdParty/Catch2/include)

# Configurations
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/bin/Debug-${PLATFORM}/socklib-tests)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/bin/Release-${PLATFORM}/socklib-tests)

# Define socklib static library
add_library(socklib STATIC
        include/socklib/Socket.h
        src/Socket.cpp
)

target_include_directories(socklib PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(socklib PUBLIC cxx_std_20)

# Compiler settings for socklib
target_compile_definitions(socklib PRIVATE $<$<CONFIG:Debug>:DEBUG_BUILD>)
target_compile_options(socklib PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
)

# Define socklib-tests executable
add_executable(socklib-tests
        tests/src/SocketTests.cpp
        tests/src/main.cpp
)

target_include_directories(socklib-tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CATCH_INCLUDE_DIR}
)

target_link_libraries(socklib-tests PRIVATE socklib)
target_compile_definitions(socklib-tests PRIVATE CATCH_CONFIG_NO_POSIX_SIGNALS)

# Compiler settings for socklib-tests
target_compile_definitions(socklib-tests PRIVATE $<$<CONFIG:Debug>:DEBUG_BUILD>)
target_compile_options(socklib-tests PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-O3>
)

# Toolset and system-specific settings
if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_COMPILER "clang++")
endif()
