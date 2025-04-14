# Find the Cmocka testing library
# Requires cmocka development libraries to be installed
# (e.g., libcmocka-dev on Debian/Ubuntu)
# If find_package doesn't work, you might need to provide FindCMocka.cmake
# or use pkg-config (find_package(PkgConfig REQUIRED), pkg_check_modules(CMOCKA REQUIRED cmocka))
find_package(cmocka REQUIRED)

# Find SQLite3 again (tests link against app code that uses it)
find_package(SQLite3 REQUIRED)

# Define Test Source Files
set(TEST_SOURCES
    test_main.c
    # Add separate test files for better organization:
    # test_db.c
    # test_queries.c
    # test_auth.c
)

# Define Application Source Files needed by the tests
# (Exclude main.c from the application sources)
set(APP_SOURCES_FOR_TESTS
    ../src/db.c
    ../src/queries.c
    ../src/auth.c
)

# Define the test executable
add_executable(run_tests ${TEST_SOURCES} ${APP_SOURCES_FOR_TESTS})

# Include directories for tests (to find app headers)
target_include_directories(run_tests PRIVATE ../includes)

# Link required libraries to the test executable
target_link_libraries(run_tests PRIVATE
    cmocka::cmocka      # Link cmocka (modern CMake target)
    SQLite::SQLite3     # Link SQLite3
    m                   # Link math library
)

# Add the test executable to CTest registry
# The name "PerfumeBazaarTests" is how you run it with `ctest -R PerfumeBazaarTests`
add_test(NAME PerfumeBazaarTests COMMAND run_tests)

# Optional: Set test properties, like working directory if needed
# set_tests_properties(PerfumeBazaarTests PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})