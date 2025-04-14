# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name and Language
project(PerfumeBazaar C)

# Set C Standard (C11 is a good choice)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF) # Prefer standard C over GNU extensions

# Compiler Flags
# Common flags for all build types
set(COMMON_C_FLAGS "-Wall -Wextra -pedantic")
# Debug flags (add -g for debugging symbols)
set(CMAKE_C_FLAGS_DEBUG "-g ${COMMON_C_FLAGS}")
# Release flags (add optimizations)
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG ${COMMON_FLAGS}") # -O3 is aggressive optimization
# RelWithDebInfo (Release with Debug Info)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG ${COMMON_FLAGS}")
# MinSizeRel (Smallest size release)
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG ${COMMON_FLAGS}")


# Set output directories for binaries (relative to build directory)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories (make headers in 'includes' available)
include_directories(includes)

# Find SQLite3 library dependency
# This requires SQLite3 development libraries to be installed on the system
# (e.g., libsqlite3-dev on Debian/Ubuntu, sqlite-devel on Fedora/CentOS)
find_package(SQLite3 REQUIRED)

# Define Application Source Files
set(APP_SOURCES
    src/main.c
    src/db.c
    src/queries.c
    src/auth.c
)

# Define the main executable
add_executable(PerfumeBazaar ${APP_SOURCES})

add_custom_command(TARGET PerfumeBazaar POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/docs/database_schema.sql" # Откуда копировать
        "$<TARGET_FILE_DIR:PerfumeBazaar>/database_schema.sql" # Куда копировать (рядом с exe)
    COMMENT "Copying database schema to output directory"
)

add_custom_command(TARGET PerfumeBazaar POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/docs/seed_data.sql"      # <<< Имя нового файла
        "$<TARGET_FILE_DIR:PerfumeBazaar>/seed_data.sql" # <<< Имя нового файла
    COMMENT "Copying database seed data to output directory"
)

# Link required libraries to the executable
# SQLite::SQLite3 is the modern CMake target name provided by find_package(SQLite3)
# m is the math library (often needed, especially with C standard library functions)
target_link_libraries(PerfumeBazaar PRIVATE SQLite::SQLite3 m)

# Enable testing support with CTest
enable_testing()

# Add the tests subdirectory (CMake will look for tests/CMakeLists.txt)
add_subdirectory(tests)

# Optional: Install rules (if you want to install the application)
# install(TARGETS PerfumeBazaar DESTINATION bin)
# install(FILES docs/database_schema.sql DESTINATION share/PerfumeBazaar)

# Print configuration summary
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binaries output to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")