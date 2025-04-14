#include "db.h"
#include <sqlite3.h>
#include <stdio.h>

sqlite3 *db;

int open_db(const char *filename) {
  int rc = sqlite3_open(filename, &db);
  if (rc) {
    fprintf(stderr, "Не удалось открыть БД: %s\n", sqlite3_errmsg(db));
    return rc;
  }
  return 0;
}

void close_db() { sqlite3_close(db); }

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  for (int i = 0; i < argc; i++) {
    printf("%s: %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

void execute_query(const char *query) {
  char *errMsg = NULL;
  int rc = sqlite3_exec(db, query, callback, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Ошибка SQL: %s\n", errMsg);
    sqlite3_free(errMsg);
  }
}