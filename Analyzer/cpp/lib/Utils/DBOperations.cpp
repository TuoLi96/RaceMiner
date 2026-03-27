#include "Utils/DBOperations.h"

using namespace std;

string get_column_text(sqlite3_stmt *stmt, int index) {
	const unsigned char *text = sqlite3_column_text(stmt, index);
	return text ? reinterpret_cast<const char*>(text) : "";
}

int get_column_int(sqlite3_stmt *stmt, int index) {
	return sqlite3_column_int(stmt, index);
}
