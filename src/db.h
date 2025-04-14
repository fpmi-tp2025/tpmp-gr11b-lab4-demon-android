#ifndef DB_H
#define DB_H

#include <sqlite3.h>

extern sqlite3 *db;

int open_db(const char *filename);
void close_db();
void execute_query(const char *query);
int callback(void *NotUsed, int argc, char **argv, char **azColName);

#endif