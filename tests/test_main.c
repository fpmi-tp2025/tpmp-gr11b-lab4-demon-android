// tests/test_main.c

#include "../includes/auth.h" // Correct path
#include "../includes/db.h"   // Correct path

#include <setjmp.h> // For jmp_buf (required BEFORE cmocka.h)
#include <stdio.h>  // For FILE, fopen, fprintf, fclose, remove, printf

#include <cmocka.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h> // For system() or file operations if needed
#include <unistd.h> // For access()

// Test database file name
#define TEST_DB_FILE "test_perfume_market.db"
#define TEST_SCHEMA_FILE "test_schema.sql" // Use a copy or specific test schema

// --- Setup and Teardown ---

// --- ИСПРАВЛЕННАЯ create_test_schema_file ---
static void create_test_schema_file() {
  FILE *fp = fopen(TEST_SCHEMA_FILE, "w");
  if (fp) {
    fprintf(fp, "-- Test Schema --\n");
    fprintf(fp, "PRAGMA foreign_keys = ON;\n\n");

    // Brokers table
    fprintf(fp,
            "CREATE TABLE IF NOT EXISTS Brokers (\n"); // Added IF NOT EXISTS
    fprintf(fp, "    surname TEXT PRIMARY KEY NOT NULL,\n");
    fprintf(fp, "    address TEXT,\n");
    fprintf(fp, "    birth_year INTEGER\n");
    fprintf(fp, ");\n\n");

    // Suppliers table (Needed by Goods)
    fprintf(fp,
            "CREATE TABLE IF NOT EXISTS Suppliers (\n"); // Added IF NOT EXISTS
    fprintf(fp, "    supplier_name TEXT PRIMARY KEY NOT NULL,\n");
    fprintf(fp, "    contact_info TEXT\n");
    fprintf(fp, ");\n\n");

    // Buyers table (Needed by Deals)
    fprintf(fp, "CREATE TABLE IF NOT EXISTS Buyers (\n"); // Added IF NOT EXISTS
    fprintf(fp, "    buyer_name TEXT PRIMARY KEY NOT NULL,\n");
    fprintf(fp, "    address TEXT\n");
    fprintf(fp, ");\n\n");

    // User table (References Brokers) - !!! ВАЖНО: Создаем ПОСЛЕ Brokers !!!
    fprintf(fp, "CREATE TABLE IF NOT EXISTS Users (\n"); // Added IF NOT EXISTS
    fprintf(fp, "    user_id INTEGER PRIMARY KEY AUTOINCREMENT,\n");
    fprintf(fp, "    username TEXT UNIQUE NOT NULL,\n");
    fprintf(fp, "    password_hash TEXT NOT NULL,\n");
    fprintf(fp, "    role TEXT NOT NULL CHECK(role IN ('admin', 'broker')),\n");
    fprintf(fp, "    broker_surname_fk TEXT,\n");
    fprintf(fp, "    FOREIGN KEY (broker_surname_fk) REFERENCES "
                "Brokers(surname) ON DELETE SET NULL ON UPDATE CASCADE\n");
    fprintf(fp, ");\n\n");

    // Goods table (References Suppliers) - !!! ВАЖНО: Создаем ПОСЛЕ Suppliers
    // !!!
    fprintf(fp, "CREATE TABLE IF NOT EXISTS Goods (\n"); // Added IF NOT EXISTS
    fprintf(fp, "    good_id INTEGER PRIMARY KEY AUTOINCREMENT,\n");
    fprintf(fp, "    name TEXT NOT NULL,\n");
    fprintf(fp, "    type_of_good TEXT,\n");
    fprintf(fp, "    price REAL NOT NULL CHECK(price > 0),\n");
    fprintf(fp, "    supplier_name_fk TEXT NOT NULL,\n");
    fprintf(fp, "    expiry_date TEXT,\n");
    fprintf(fp, "    quantity INTEGER NOT NULL CHECK(quantity >= 0),\n");
    fprintf(fp,
            "    FOREIGN KEY (supplier_name_fk) REFERENCES "
            "Suppliers(supplier_name) ON DELETE RESTRICT ON UPDATE CASCADE,\n");
    fprintf(fp, "    UNIQUE (name, supplier_name_fk)\n");
    fprintf(fp, ");\n\n");

    // Deals table (References Brokers, Buyers, Goods) - !!! ВАЖНО: Создаем
    // ПОСЛЕ Brokers, Buyers, Goods !!!
    fprintf(fp, "CREATE TABLE IF NOT EXISTS Deals (\n"); // Added IF NOT EXISTS
    fprintf(fp, "    deal_id INTEGER PRIMARY KEY AUTOINCREMENT,\n");
    fprintf(fp, "    deal_date TEXT NOT NULL,\n");
    fprintf(fp, "    good_name_fk TEXT NOT NULL,\n");
    fprintf(fp, "    supplier_name_fk TEXT NOT NULL,\n");
    fprintf(fp, "    type_of_good TEXT,\n");
    fprintf(fp,
            "    sell_quantity INTEGER NOT NULL CHECK(sell_quantity > 0),\n");
    fprintf(fp, "    broker_surname_fk TEXT NOT NULL,\n");
    fprintf(fp, "    buyer_name_fk TEXT NOT NULL,\n");
    fprintf(fp, "    FOREIGN KEY (broker_surname_fk) REFERENCES "
                "Brokers(surname) ON DELETE RESTRICT ON UPDATE CASCADE,\n");
    fprintf(fp, "    FOREIGN KEY (buyer_name_fk) REFERENCES Buyers(buyer_name) "
                "ON DELETE RESTRICT ON UPDATE CASCADE,\n");
    fprintf(
        fp,
        "    FOREIGN KEY (good_name_fk, supplier_name_fk) REFERENCES "
        "Goods(name, supplier_name_fk) ON DELETE RESTRICT ON UPDATE CASCADE\n");
    fprintf(fp, ");\n\n");

    // BrokerStats table (References Brokers) - !!! ВАЖНО: Создаем ПОСЛЕ Brokers
    // !!!
    fprintf(
        fp,
        "CREATE TABLE IF NOT EXISTS BrokerStats (\n"); // Added IF NOT EXISTS
    fprintf(fp, "    stat_id INTEGER PRIMARY KEY AUTOINCREMENT,\n");
    fprintf(fp, "    broker_surname_fk TEXT NOT NULL UNIQUE,\n");
    fprintf(fp, "    total_sold_units INTEGER DEFAULT 0,\n");
    fprintf(fp, "    total_deal_sum REAL DEFAULT 0.0,\n");
    fprintf(fp, "    last_updated TEXT,\n");
    fprintf(fp, "    FOREIGN KEY (broker_surname_fk) REFERENCES "
                "Brokers(surname) ON DELETE CASCADE ON UPDATE CASCADE\n");
    fprintf(fp, ");\n\n");

    // Optional: Add initial Users/Brokers for auth tests if needed
    fprintf(fp,
            "INSERT OR IGNORE INTO Brokers (surname) VALUES ('TestBroker');\n");
    fprintf(fp, "INSERT OR IGNORE INTO Users (username, password_hash, role) "
                "VALUES ('testuser', 'hashed_testpass', 'admin');\n");

    fclose(fp);
  } else {
    // fail_msg is cmocka specific, ensure cmocka.h is included before use
    // Including stdio.h for fprintf to stderr as a fallback
    fprintf(
        stderr,
        "!!! CRITICAL TEST SETUP ERROR: Could not create test schema file %s\n",
        TEST_SCHEMA_FILE);
    // We might want to exit or explicitly fail here
    // For cmocka, calling fail_msg is preferred if possible
    fail_msg("Could not create test schema file %s", TEST_SCHEMA_FILE);
  }
}
// --- КОНЕЦ ИСПРАВЛЕННОЙ ВЕРСИИ ---

// --- Setup and Teardown for tests using the database ---
static int setup_db(void **state) {
  (void)state; // Not using state for simple tests

  // Ensure no previous test DB exists
  remove(TEST_DB_FILE);
  // Create the necessary schema file for tests
  create_test_schema_file();

  // Open the test database
  if (open_db(TEST_DB_FILE) != 0) {
    fprintf(stderr, "!!! TEST SETUP FAILED: open_db failed for %s\n",
            TEST_DB_FILE);
    return -1; // Indicate setup failure
  }

  // Initialize schema using the generated test schema file
  if (init_tables_if_needed(TEST_SCHEMA_FILE) != 0) {
    fprintf(stderr,
            "!!! TEST SETUP FAILED: init_tables_if_needed failed for %s\n",
            TEST_SCHEMA_FILE);
    close_db();               // Clean up DB connection
    remove(TEST_DB_FILE);     // Clean up DB file
    remove(TEST_SCHEMA_FILE); // Clean up schema file
    return -1;                // Indicate setup failure
  }

  // Optionally preload some common test data here using execute_non_query
  // Example:
  // execute_non_query("INSERT INTO Brokers (surname, address, birth_year)
  // VALUES ('SetupBroker', '1 Setup St', 1995);");

  return 0; // Success
}

static int teardown_db(void **state) {
  (void)state;
  close_db();
  remove(TEST_DB_FILE);
  remove(TEST_SCHEMA_FILE); // Clean up the generated test schema file
  return 0;
}

// --- Tests for db.c (Example placeholders) ---
// These might be better placed in a dedicated test_db.c

static void test_open_close_db_success(void **state) {
  (void)state;
  // The setup_db function already opens the database.
  // This test could verify internal state if possible, or simply
  // rely on setup/teardown succeeding as implicit test.
  // Let's test closing and reopening within the test itself for clarity.
  close_db();
  assert_null(db); // Check internal state after close

  int rc = open_db(TEST_DB_FILE); // Re-open
  assert_int_equal(rc, 0);
  assert_non_null(db);
  // Teardown will handle the final close.
}

static void test_init_tables_run_once(void **state) {
  (void)state;
  // setup_db already called init_tables_if_needed successfully.
  // Call it again - it should detect tables exist and not re-run the script.
  printf("--- Running test: %s ---\n", __func__); // Identify test
  int rc = init_tables_if_needed(TEST_SCHEMA_FILE);
  // Should return 0 because table 'Users' exists from the setup phase.
  assert_int_equal(rc, 0);
  // Add more checks if needed (e.g., verify no data was wiped)
}

static void test_execute_non_query_success(void **state) {
  (void)state;
  printf("--- Running test: %s ---\n", __func__); // Identify test
  const char *insert_sql = "INSERT INTO Brokers (surname, address, birth_year) "
                           "VALUES ('Smith', '2 Elm St', 1985);";
  int rc = execute_non_query(insert_sql);
  assert_int_equal(rc, SQLITE_OK);
  // Optionally verify the insert using prepare/step/column or another select
  // query
}

static void test_execute_non_query_fail_syntax(void **state) {
  (void)state;
  printf("--- Running test: %s ---\n", __func__); // Identify test
  const char *bad_sql =
      "INSERTT INTO Brokers VALUES ('Jones');"; // Intentional typo
  int rc = execute_non_query(bad_sql);
  // Expecting an error (e.g., SQLITE_ERROR from prepare)
  assert_int_not_equal(rc, SQLITE_OK);
  assert_int_not_equal(rc, SQLITE_DONE); // Should fail before step
}

static void test_execute_select_query_found(void **state) {
  (void)state;
  printf("--- Running test: %s ---\n", __func__); // Identify test
  // Insert known data first
  execute_non_query("INSERT OR IGNORE INTO Brokers (surname, address) VALUES "
                    "('SelectTest', '123 Select St');");
  const char *select_sql =
      "SELECT address FROM Brokers WHERE surname = 'SelectTest';";
  // For better testing, capture output or use prepare/step/column.
  // Here, we just check if sqlite3_exec runs without error.
  int rc = execute_select_query(select_sql);
  assert_int_equal(rc, SQLITE_OK);
}

static void test_execute_select_query_not_found(void **state) {
  (void)state;
  printf("--- Running test: %s ---\n", __func__); // Identify test
  const char *select_sql =
      "SELECT address FROM Brokers WHERE surname = 'NoSuchBroker';";
  int rc = execute_select_query(select_sql);
  // Query is valid, just returns no rows. sqlite3_exec should return SQLITE_OK.
  assert_int_equal(rc, SQLITE_OK);
}

// --- Placeholder tests for queries.c ---
// These should be moved to test_queries.c and implemented fully

static void test_query_sales_summary(void **state) {
  (void)state;
  printf("--- Running test: %s (Placeholder) ---\n", __func__);
  assert_true(1);
}
static void test_query_buyers_by_good(void **state) {
  (void)state;
  printf("--- Running test: %s (Placeholder) ---\n", __func__);
  assert_true(1);
}
static void test_query_most_popular(void **state) {
  (void)state;
  printf("--- Running test: %s (Placeholder) ---\n", __func__);
  assert_true(1);
}

// --- Placeholder tests for auth.c ---
// These should be moved to test_auth.c and implemented fully

static void test_auth_login_success(void **state) {
  (void)state;
  printf("--- Running test: %s (Placeholder) ---\n", __func__);
  assert_true(1);
}
static void test_auth_login_fail_password(void **state) {
  (void)state;
  printf("--- Running test: %s (Placeholder) ---\n", __func__);
  assert_true(1);
}
static void test_auth_login_fail_user(void **state) {
  (void)state;
  printf("--- Running test: %s (Placeholder) ---\n", __func__);
  assert_true(1);
}

// --- Main Test Runner ---
int main(void) {
  // Define test groups
  // Group for tests primarily exercising db.c functions
  const struct CMUnitTest db_tests[] = {
      cmocka_unit_test(test_open_close_db_success),
      cmocka_unit_test(test_init_tables_run_once),
      cmocka_unit_test(test_execute_non_query_success),
      cmocka_unit_test(test_execute_non_query_fail_syntax),
      cmocka_unit_test(test_execute_select_query_found),
      cmocka_unit_test(test_execute_select_query_not_found),
      // Add more tests specifically validating db.c logic here
  };

  // Group for tests primarily exercising queries.c functions (Placeholders)
  const struct CMUnitTest query_tests[] = {
      cmocka_unit_test(test_query_sales_summary),
      cmocka_unit_test(test_query_buyers_by_good),
      cmocka_unit_test(test_query_most_popular),
      // Add more tests specifically validating queries.c logic here
  };

  // Group for tests primarily exercising auth.c functions (Placeholders)
  const struct CMUnitTest auth_tests[] = {
      cmocka_unit_test(test_auth_login_success),
      cmocka_unit_test(test_auth_login_fail_password),
      cmocka_unit_test(test_auth_login_fail_user),
      // Add more tests specifically validating auth.c logic here
  };

  // Run tests with setup/teardown for each group
  int failed = 0;
  printf("\n--- Running DB Tests ---\n");
  failed += cmocka_run_group_tests(db_tests, setup_db, teardown_db);

  printf("\n--- Running Query Tests (Placeholders) ---\n");
  failed += cmocka_run_group_tests(query_tests, setup_db, teardown_db);

  printf("\n--- Running Auth Tests (Placeholders) ---\n");
  failed += cmocka_run_group_tests(auth_tests, setup_db, teardown_db);

  // You might have other test groups or standalone tests here

  return failed; // Return number of failed tests
}