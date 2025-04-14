# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name and Language
project(PerfumeBazaar C)

# Set C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS OFF)

# --- Опция для включения покрытия ---
option(ENABLE_COVERAGE "Enable code coverage flags and options" OFF)

# Compiler Flags
set(COMMON_C_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_C_FLAGS_DEBUG "-g ${COMMON_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG ${COMMON_C_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG ${COMMON_C_FLAGS}")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG ${COMMON_C_FLAGS}")

# --- Применяем флаги покрытия, если опция включена ---
if(ENABLE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        # Флаги для GCC/Clang (создают .gcno и инструментируют код для .gcda)
        set(COVERAGE_COMPILE_FLAGS "--coverage")
        set(COVERAGE_LINK_FLAGS "--coverage")
        # Отключаем оптимизацию для более точного покрытия по строкам
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
        set(CMAKE_C_FLAGS_RELEASE "-O0") # Покрытие обычно измеряют для Debug сборок
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O0 -g")
        set(CMAKE_C_FLAGS_MINSIZEREL "-O0")
        message(STATUS "Code coverage enabled with flags: ${COVERAGE_COMPILE_FLAGS}")
    else()
        message(WARNING "Code coverage not supported for this compiler.")
        set(COVERAGE_COMPILE_FLAGS "")
        set(COVERAGE_LINK_FLAGS "")
    endif()
endif()
# --- Конец добавления флагов покрытия ---

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(includes)

# Find SQLite3 library
find_package(SQLite3 REQUIRED)

# --- Собираем основной код в СТАТИЧЕСКУЮ БИБЛИОТЕКУ ---
set(APP_SOURCES
    src/db.c
    src/queries.c
    src/auth.c
    # НЕ ВКЛЮЧАЕМ src/main.c сюда!
)
add_library(PerfumeBazaarLib STATIC ${APP_SOURCES})
# Применяем флаги покрытия к библиотеке
if(ENABLE_COVERAGE)
    target_compile_options(PerfumeBazaarLib PRIVATE ${COVERAGE_COMPILE_FLAGS})
    target_link_options(PerfumeBazaarLib INTERFACE ${COVERAGE_LINK_FLAGS}) # Linker flags needed by objects
endif()
# --- Конец сборки библиотеки ---

# --- Собираем основное приложение ---
add_executable(PerfumeBazaar src/main.c) # Только main.c
# Линкуем основное приложение с библиотекой и зависимостями
target_link_libraries(PerfumeBazaar PRIVATE PerfumeBazaarLib SQLite::SQLite3 m)
# --- Конец сборки приложения ---

# --- Копирование файлов схемы и данных (остается как было) ---
add_custom_command(TARGET PerfumeBazaar POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/docs/database_schema.sql"
        "$<TARGET_FILE_DIR:PerfumeBazaar>/database_schema.sql"
    COMMENT "Copying database schema to output directory"
)
add_custom_command(TARGET PerfumeBazaar POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/docs/seed_data.sql"
        "$<TARGET_FILE_DIR:PerfumeBazaar>/seed_data.sql"
    COMMENT "Copying database seed data to output directory"
)
# --- Конец копирования ---

# Enable testing support with CTest
enable_testing()

# Add the tests subdirectory
add_subdirectory(tests)

# Print configuration summary
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Coverage enabled: ${ENABLE_COVERAGE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binaries output to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")