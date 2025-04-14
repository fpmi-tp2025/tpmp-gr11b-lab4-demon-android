#include "../includes/db.h" // Correct path
#include <ctype.h>          // For isspace
#include <errno.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For strcmp, strlen

sqlite3 *db = NULL;

// --- open_db ---
int open_db(const char *filename) {
  if (db != NULL) {
    fprintf(stderr, "DEBUG: Database already open.\n");
    return 0;
  }
  printf("DEBUG: Attempting to open/create database: %s\n", filename);
  int rc = sqlite3_open_v2(filename, &db,
                           SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "!!! sqlite3_open_v2 failed: %s (rc=%d)\n",
            sqlite3_errmsg(db), rc);
    sqlite3_close(db);
    db = NULL;
    return rc;
  }
  printf("DEBUG: sqlite3_open_v2 succeeded. db pointer: %p\n", (void *)db);

  printf("DEBUG: Executing PRAGMA foreign_keys=ON...\n");
  // Используем execute_non_query_internal, так как execute_sql_from_file еще
  // может быть не вызван
  sqlite3_stmt *stmt = NULL;
  int rcFK =
      sqlite3_prepare_v2(db, "PRAGMA foreign_keys = ON;", -1, &stmt, NULL);
  if (rcFK == SQLITE_OK) {
    rcFK = sqlite3_step(stmt);
    if (rcFK != SQLITE_DONE) {
      fprintf(stderr,
              "!!! Failed to step 'PRAGMA foreign_keys = ON;': %s (rc=%d)\n",
              sqlite3_errmsg(db), rcFK);
    }
    rcFK = sqlite3_finalize(stmt); // Всегда финализируем
    if (rcFK != SQLITE_OK) {
      fprintf(
          stderr,
          "!!! Failed to finalize 'PRAGMA foreign_keys = ON;': %s (rc=%d)\n",
          sqlite3_errmsg(db), rcFK);
    } else {
      rcFK = SQLITE_OK; // Если step был DONE и finalize OK, то все хорошо
    }
  } else {
    fprintf(stderr,
            "!!! Failed to prepare 'PRAGMA foreign_keys = ON;': %s (rc=%d)\n",
            sqlite3_errmsg(db), rcFK);
  }

  if (rcFK != SQLITE_OK) {
    fprintf(stderr,
            "!!! Failed to enable foreign keys (rc=%d). Closing database.\n",
            rcFK);
    sqlite3_close(db);
    db = NULL;
    return rcFK;
  }
  printf("DEBUG: 'PRAGMA foreign_keys = ON;' executed successfully.\n");

  printf("Database opened successfully: %s\n", filename);
  return 0;
}

// --- close_db ---
void close_db() {
  if (db) {
    printf("DEBUG: Closing database...\n");
    int rc = sqlite3_close(db);
    if (rc == SQLITE_OK) {
      printf("Database closed successfully.\n");
    } else {
      fprintf(stderr, "!!! Error closing database: %s (rc=%d)\n",
              sqlite3_errmsg(db), rc);
    }
    db = NULL;
  } else {
    printf("DEBUG: Database already closed or never opened.\n");
  }
}

// --- default_callback (for SELECT) ---
int default_callback(void *NotUsed, int argc, char **argv, char **azColName) {
  (void)NotUsed;
  printf("--- Query Result Row ---\n");
  for (int i = 0; i < argc; i++) {
    printf("  %-20s: %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("------------------------\n");
  return 0;
}

// --- execute_non_query (uses prepare/step/finalize) ---
int execute_non_query(const char *query) {
  if (!db) {
    fprintf(stderr, "!!! execute_non_query: Database not open.\n");
    return SQLITE_ERROR;
  }
  if (!query || query[0] == '\0') {
    // It's okay to receive empty queries sometimes after parsing, just ignore
    // them. fprintf(stderr, "!!! execute_non_query: Received empty query.\n");
    return SQLITE_OK; // Treat empty query as success (no-op)
  }

  sqlite3_stmt *stmt = NULL;
  int rc_prepare = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

  if (rc_prepare != SQLITE_OK) {
    fprintf(stderr, "!!! SQL prepare error (%d) for query [%s]: %s\n",
            rc_prepare, query, sqlite3_errmsg(db));
    sqlite3_finalize(stmt); // Finalize even if NULL or error
    return rc_prepare;
  }
  if (!stmt) {
    // This case means the input string contained only comments or whitespace
    // printf("DEBUG: SQL prepare resulted in NULL statement (likely
    // comment/whitespace): [%s]\n", query);
    return SQLITE_OK; // Valid outcome, just nothing to execute
  }

  int rc_step = sqlite3_step(stmt);
  if (rc_step != SQLITE_DONE) {
    fprintf(stderr, "!!! SQL step error (%d) for query [%s]: %s\n", rc_step,
            query, sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return rc_step;
  }

  int rc_finalize = sqlite3_finalize(stmt);
  if (rc_finalize != SQLITE_OK) {
    fprintf(stderr, "!!! SQL finalize error (%d) for query [%s]: %s\n",
            rc_finalize, query, sqlite3_errmsg(db));
    return rc_finalize;
  }
  return SQLITE_OK;
}

// --- execute_select_query (uses sqlite3_exec) ---
int execute_select_query(const char *query) {
  if (!db) {
    fprintf(stderr, "!!! execute_select_query: Database not open.\n");
    return SQLITE_ERROR;
  }
  char *errMsg = NULL;
  printf("DEBUG: Executing SELECT: %s\n", query);
  int rc = sqlite3_exec(db, query, default_callback, NULL,
                        &errMsg); // No need for callback_arg here

  if (rc != SQLITE_OK) {
    fprintf(stderr, "!!! SQL SELECT error (%d): %s\nQuery: %s\n", rc, errMsg,
            query);
    sqlite3_free(errMsg);
    return rc;
  }
  printf("--- SELECT query finished ---\n");
  return SQLITE_OK;
}

// --- Simplified execute_sql_from_file ---
int execute_sql_from_file(const char *filename) {
  if (!db) {
    fprintf(stderr, "!!! Database not open for executing SQL file.\n");
    return SQLITE_ERROR;
  }
  printf("DEBUG: Attempting to open SQL file: %s\n", filename);
  FILE *fp = fopen(filename, "rb"); // Open in binary read mode
  if (!fp) {
    perror("!!! Cannot open SQL file");
    fprintf(stderr, "!!! Failed to open SQL file at path: %s\n", filename);
    return 1; // Indicate file error
  }
  printf("DEBUG: SQL file opened successfully: %s\n", filename);

  // Get file size
  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (file_size <= 0) {
    fprintf(stderr, "!!! SQL file is empty or size error: %s\n", filename);
    fclose(fp);
    return SQLITE_OK; // Empty file is not an error in itself
  }

  // Allocate buffer for the whole file + null terminator
  char *sql_buffer = (char *)malloc(file_size + 1);
  if (!sql_buffer) {
    fprintf(stderr,
            "!!! Failed to allocate memory (%ld bytes) to read SQL file: %s\n",
            file_size + 1, filename);
    fclose(fp);
    return SQLITE_ERROR; // Memory allocation error
  }

  // Read the whole file
  size_t bytes_read = fread(sql_buffer, 1, file_size, fp);
  fclose(fp); // Close file immediately after reading

  if (bytes_read != (size_t)file_size) {
    fprintf(
        stderr,
        "!!! Failed to read entire SQL file (%zu bytes read out of %ld): %s\n",
        bytes_read, file_size, filename);
    free(sql_buffer);
    return SQLITE_ERROR; // File read error
  }

  // Null-terminate the buffer
  sql_buffer[file_size] = '\0';

  // Execute the entire buffer using sqlite3_exec
  printf(
      "DEBUG: Executing entire SQL script from buffer (size: %ld bytes)...\n",
      file_size);
  char *errMsg = NULL;
  int rc = sqlite3_exec(db, sql_buffer, NULL, NULL,
                        &errMsg); // No callback needed here

  free(sql_buffer); // Free the buffer

  if (rc != SQLITE_OK) {
    fprintf(stderr, "!!! Error executing SQL script: %s (rc=%d)\n",
            errMsg ? errMsg : "Unknown error", rc);
    sqlite3_free(errMsg);
    return rc; // Return the SQLite error code
  }

  printf("DEBUG: SQL script executed successfully via sqlite3_exec.\n");
  return SQLITE_OK;
}

// --- table_exists_callback ---
static int table_exists_callback(void *data, int argc, char **argv,
                                 char **azColName) {
  int *found = (int *)data;
  *found = 1;
  (void)argc;
  (void)argv;
  (void)azColName;
  return 0;
}

// --- table_exists ---
int table_exists(const char *table_name) {
  if (!db) {
    fprintf(stderr, "!!! table_exists: Database not open.\n");
    return -1;
  }
  char query[256];
  snprintf(query, sizeof(query),
           "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';",
           table_name);
  int found = 0;
  char *errMsg = NULL;
  printf("DEBUG: Executing table existence check for: %s\n", table_name);
  int rc = sqlite3_exec(db, query, table_exists_callback, &found, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr,
            "!!! SQL error checking table existence for '%s': %s (rc=%d)\n",
            table_name, errMsg, rc);
    sqlite3_free(errMsg);
    return -1;
  }
  printf("DEBUG: Table '%s' found status (0=No, 1=Yes): %d\n", table_name,
         found);
  return found;
}

// --- init_tables_if_needed ---
int init_tables_if_needed(const char *schema_file) {
  printf("DEBUG: Checking database schema presence...\n");
  int exists = table_exists("Users");

  if (exists == -1) {
    fprintf(stderr, "!!! DEBUG: Failed to check if table 'Users' exists. "
                    "Aborting schema initialization.\n");
    return -1;
  }
  printf("DEBUG: Initial check for 'Users' table returned: %d\n", exists);

  if (exists == 0) {
    printf("DEBUG: Table 'Users' not found. Attempting to initialize database "
           "from %s...\n",
           schema_file);
    printf(
        "DEBUG: --- Calling execute_sql_from_file (simplified version) ---\n");
    int rc_exec = execute_sql_from_file(schema_file);
    printf("DEBUG: --- execute_sql_from_file returned %d ---\n", rc_exec);

    if (rc_exec != SQLITE_OK) {
      fprintf(stderr,
              "!!! DEBUG: Schema execution from '%s' failed with rc=%d. "
              "Aborting.\n",
              schema_file, rc_exec);
      return rc_exec;
    }

    printf("DEBUG: Schema script execution sequence reportedly finished OK. "
           "Verifying 'Users' table presence AGAIN...\n");
    int exists_after = table_exists("Users");
    printf("DEBUG: Verification check for 'Users' table returned: %d\n",
           exists_after);

    if (exists_after == 1) {
      printf("DEBUG: Verification successful: Table 'Users' now exists.\n");

      printf("DEBUG: --- Attempting to SEED database with initial data ---\n");
      // Предполагаем, что seed_data.sql скопирован рядом с исполняемым файлом
      int rc_exec_seed = execute_sql_from_file("seed_data.sql");
      printf("DEBUG: --- execute_sql_from_file for SEED returned %d ---\n",
             rc_exec_seed);
      if (rc_exec_seed != SQLITE_OK) {
        fprintf(
            stderr,
            "!!! WARNING: Failed to execute seed data script 'seed_data.sql' "
            "(rc=%d). Database schema created, but seeding failed.\n",
            rc_exec_seed);
        // Не возвращаем ошибку, так как схема создана, но предупреждаем
      } else {
        printf("DEBUG: Seed data script executed successfully.\n");
      }

      return 0; // Explicit Success
    } else {
      fprintf(stderr,
              "!!! CRITICAL ERROR: Schema execution finished OK BUT table "
              "'Users' STILL not found! (exists_after=%d)\n",
              exists_after);
      return -1; // Indicate critical inconsistency
    }

  } else { // exists == 1
    printf("DEBUG: Database table 'Users' seems to exist. Skipping schema "
           "execution.\n");
    return 0; // Tables already exist
  }
}