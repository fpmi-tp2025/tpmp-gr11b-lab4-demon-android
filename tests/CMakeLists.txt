# Find the Cmocka testing library
find_package(cmocka REQUIRED)

# Find SQLite3 (нужно для библиотеки PerfumeBazaarLib)
find_package(SQLite3 REQUIRED)

# Define Test Source Files
set(TEST_SOURCES
    test_main.c
    # test_db.c        # Раскомментируйте, если создали
    # test_queries.c   # Раскомментируйте, если создали
    # test_auth.c      # Раскомментируйте, если создали
)

# Define the test executable (только из тестовых исходников)
add_executable(run_tests ${TEST_SOURCES})

# Include directories for tests (чтобы найти includes/*.h)
target_include_directories(run_tests PRIVATE ../includes)

# Link required libraries to the test executable
target_link_libraries(run_tests PRIVATE
    PerfumeBazaarLib    # <<< Линкуемся с нашей библиотекой
    cmocka::cmocka
    SQLite::SQLite3
    m
)

# Add the test executable to CTest registry
add_test(NAME PerfumeBazaarTests COMMAND run_tests)

# Optional: Set test properties
# set_tests_properties(PerfumeBazaarTests PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)