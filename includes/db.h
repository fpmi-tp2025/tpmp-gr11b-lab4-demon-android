#ifndef DB_H
#define DB_H

#include <sqlite3.h>

// Global database handle (consider alternatives for larger apps)
extern sqlite3 *db;

/**
 * @brief Opens the SQLite database file.
 * @param filename Path to the database file.
 * @return 0 on success, non-zero on failure.
 */
int open_db(const char *filename);

/**
 * @brief Closes the database connection.
 */
void close_db();

/**
 * @brief Executes an SQL query that doesn't expect results rows (e.g., INSERT, UPDATE, DELETE, CREATE).
 * Prints errors to stderr.
 * @param query The SQL query string.
 * @return 0 on success, non-zero on failure.
 */
int execute_non_query(const char *query);

/**
 * @brief Executes an SQL query that returns results (SELECT).
 * Uses the default callback to print results. Prints errors to stderr.
 * @param query The SQL query string.
 * @return 0 on success, non-zero on failure.
 */
int execute_select_query(const char *query);

/**
 * @brief Default callback function for sqlite3_exec to print results.
 */
int default_callback(void *NotUsed, int argc, char **argv, char **azColName);

/**
 * @brief Reads and executes SQL commands from a file.
 * @param filename Path to the SQL script file.
 * @return 0 on success, non-zero on failure.
 */
int execute_sql_from_file(const char *filename);

/**
 * @brief Initializes database tables by executing schema script if tables don't exist.
 * @param schema_file Path to the SQL schema file (e.g., "database_schema.sql").
 * @return 0 on success, non-zero on failure.
 */
int init_tables_if_needed(const char *schema_file);

#endif // DB_H
