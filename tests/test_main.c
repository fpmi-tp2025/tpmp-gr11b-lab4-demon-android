#include "../includes/auth.h" // Correct path
#include "../includes/db.h"   // Correct path

// +++ Add required includes +++
#include <setjmp.h> // For jmp_buf (required BEFORE cmocka.h)
#include <stdio.h>  // For FILE, fopen, fprintf, fclose, remove, printf
// +++ End Additions +++

#include <cmocka.h>
// #include <setjmp.h> // Moved above cmocka.h
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h> // For system() or file operations if needed
#include <unistd.h> // For access()

// Test database file name
#define TEST_DB_FILE "test_perfume_market.db"
#define TEST_SCHEMA_FILE "test_schema.sql" // Use a copy or specific test schema

// --- Setup and Teardown ---
// Create a minimal schema for testing
static void create_test_schema_file() {
  FILE *fp = fopen(TEST_SCHEMA_FILE, "w");
  if (fp) {
    fprintf(fp, "PRAGMA foreign_keys = ON;\n");
    fprintf(fp, "CREATE TABLE Brokers (surname TEXT PRIMARY KEY, address TEXT, "
                "birth_year INTEGER);\n");
    fprintf(fp, "CREATE TABLE Goods (good_id INTEGER PRIMARY KEY , name TEXT, "
                "supplier_name_fk TEXT, price REAL, quantity INTEGER, "
                "UNIQUE(name, supplier_name_fk));\n");
    fprintf(fp, "CREATE TABLE Deals (deal_id INTEGER PRIMARY KEY, deal_date "
                "TEXT, good_name_fk TEXT, supplier_name_fk TEXT, sell_quantity "
                "INTEGER, broker_surname_fk TEXT, buyer_name_fk TEXT);\n");
    // Add other tables as needed for specific tests (Users, BrokerStats etc.)
    fclose(fp);
  } else {
    fail_msg("Could not create test schema file %s", TEST_SCHEMA_FILE);
  }
}

static int setup_db(void **state) {
  // Ensure no previous test DB exists
  remove(TEST_DB_FILE);
  create_test_schema_file();

  // Open the test database
  if (open_db(TEST_DB_FILE) != 0) {
    return -1; // Setup failed
  }
  // Initialize schema
  if (init_tables_if_needed(TEST_SCHEMA_FILE) != 0) {
    close_db(); // Clean up
    remove(TEST_DB_FILE);
    remove(TEST_SCHEMA_FILE);
    return -1; // Setup failed
  }

  // Optionally preload some common test data here using execute_non_query
  execute_non_query("INSERT INTO Brokers (surname, address, birth_year) VALUES "
                    "('TestBroker', '1 Test St', 1990);");

  *state = NULL; // Not using state for simple tests
  return 0;      // Success
}

static int teardown_db(void **state) {
  (void)state;
  close_db();
  remove(TEST_DB_FILE);
  remove(TEST_SCHEMA_FILE);
  return 0;
}

// --- Tests for db.c ---

static void test_open_close_db_success(void **state) {
  (void)state;          // Not used
  close_db();           // Ensure db is closed if setup failed before this test
  remove(TEST_DB_FILE); // Clean up before test

  // Test opening
  int rc = open_db(TEST_DB_FILE);
  assert_int_equal(rc, 0);
  assert_non_null(db); // Check internal state (if possible/needed)

  // Test closing
  close_db();
  assert_null(db); // Check internal state after close

  // Re-open to allow teardown_db to close it properly
  open_db(TEST_DB_FILE);
}

static void test_init_tables_run_once(void **state) {
  (void)state;
  // Setup already called init_tables_if_needed
  // Call it again - it should detect tables exist and not re-run the script
  int rc = init_tables_if_needed(TEST_SCHEMA_FILE);
  assert_int_equal(rc, 0); // Should return success (tables exist)
  // Ideally, check logs or mock file I/O to confirm script wasn't re-executed
}

static void test_execute_non_query_success(void **state) {
  (void)state;
  const char *insert_sql = "INSERT INTO Brokers (surname, address, birth_year) "
                           "VALUES ('Smith', '2 Elm St', 1985);";
  int rc = execute_non_query(insert_sql);
  assert_int_equal(rc, SQLITE_OK);
  // Optionally verify the insert with a select
}

static void test_execute_non_query_fail_syntax(void **state) {
  (void)state;
  const char *bad_sql =
      "INSERTT INTO Brokers VALUES ('Jones');"; // Intentional typo
  int rc = execute_non_query(bad_sql);
  assert_int_not_equal(rc, SQLITE_OK); // Should fail
}

static void test_execute_select_query_found(void **state) {
  (void)state;
  // Assumes setup_db inserted 'TestBroker'
  const char *select_sql =
      "SELECT address FROM Brokers WHERE surname = 'TestBroker';";
  // We can't easily check the *output* of default_callback here.
  // For better testing, use prepared statements and step through results,
  // or use a custom callback that sets a flag or stores data.
  // For now, just check if the execution succeeded without SQL errors.
  int rc = execute_select_query(select_sql);
  assert_int_equal(rc, SQLITE_OK);
}

static void test_execute_select_query_not_found(void **state) {
  (void)state;
  const char *select_sql =
      "SELECT address FROM Brokers WHERE surname = 'NoSuchBroker';";
  int rc = execute_select_query(select_sql);
  assert_int_equal(rc,
                   SQLITE_OK); // Query itself is valid, just returns no rows
  // Check logs manually or use a custom callback to verify no data was printed.
}

// --- Placeholder tests for queries.c ---
// Need separate file (test_queries.c) and mocking or preloaded data

static void test_query_sales_summary(void **state) {
  (void)state;
  assert_true(1);
} // TODO
static void test_query_buyers_by_good(void **state) {
  (void)state;
  assert_true(1);
} // TODO
static void test_query_most_popular(void **state) {
  (void)state;
  assert_true(1);
} // TODO

// --- Placeholder tests for auth.c ---
// Need separate file (test_auth.c) and mocking or preloaded data

static void test_auth_login_success(void **state) {
  (void)state;
  assert_true(1);
} // TODO
static void test_auth_login_fail_password(void **state) {
  (void)state;
  assert_true(1);
} // TODO
static void test_auth_login_fail_user(void **state) {
  (void)state;
  assert_true(1);
} // TODO

int main(void) {
  // Define test groups
  const struct CMUnitTest db_tests[] = {
      cmocka_unit_test(test_open_close_db_success),
      cmocka_unit_test(test_init_tables_run_once),
      cmocka_unit_test(test_execute_non_query_success),
      cmocka_unit_test(test_execute_non_query_fail_syntax),
      cmocka_unit_test(test_execute_select_query_found),
      cmocka_unit_test(test_execute_select_query_not_found),
      // Add more db.c tests here (at least 3 total needed for db.c)
  };

  const struct CMUnitTest query_tests[] = {
      cmocka_unit_test(test_query_sales_summary),
      cmocka_unit_test(test_query_buyers_by_good),
      cmocka_unit_test(test_query_most_popular),
      // Add more tests for EACH function in queries.c (at least 3 total needed
      // for queries.c)
  };

  const struct CMUnitTest auth_tests[] = {
      cmocka_unit_test(test_auth_login_success),
      cmocka_unit_test(test_auth_login_fail_password),
      cmocka_unit_test(test_auth_login_fail_user),
      // Add more tests for auth.c (at least 3 total needed for auth.c)
  };

  // Run tests
  int failed = 0;
  printf("\n--- Running DB Tests ---\n");
  // Use group setup/teardown for tests needing the DB connection
  failed += cmocka_run_group_tests(db_tests, setup_db, teardown_db);

  printf("\n--- Running Query Tests (Placeholders) ---\n");
  // Query tests likely need DB setup too
  failed +=
      cmocka_run_group_tests(query_tests, setup_db, teardown_db); // Or mock DB

  printf("\n--- Running Auth Tests (Placeholders) ---\n");
  // Auth tests likely need DB setup too
  failed +=
      cmocka_run_group_tests(auth_tests, setup_db, teardown_db); // Or mock DB

  // Add tests for main.c if applicable (might be harder, integration tests)

  return failed;
}