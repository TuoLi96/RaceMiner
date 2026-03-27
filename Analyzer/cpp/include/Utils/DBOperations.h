#ifndef _DB_OPERATION_H_
#define _DB_OPERATION_H_

#include <sqlite3.h>
#include <string>

std::string get_column_text(sqlite3_stmt *stmt, int index);
int get_column_int(sqlite3_stmt *stmt, int index);

#endif
