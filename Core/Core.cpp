// Core.cpp : Defines the functions for the static library.
//
// This is a standard file header comment. It describes the file's purpose.

#include "pch.h"           // Precompiled header, speeds up compilation.
#include "framework.h"     // Project-specific header, may include common definitions.
#include <sqlite3.h>       // SQLite C API header, needed for database operations.

// This is a library function
void fnCore()
{
    // SQL statement to create the 'users' table if it doesn't exist.
    // 'username' is the primary key, 'password' and 'salt' are required fields.
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY, "
        "password TEXT NOT NULL, "
        "salt TEXT NOT NULL"
        ");";

    sqlite3* db; // Pointer to the SQLite database connection.

    // Opens (or creates) the database file 'example.db'.
    // If the file doesn't exist, SQLite will create it.
    sqlite3_open("FitApp.db", &db);

    // Executes the SQL statement to create the table.
    // No callback function is used (nullptr), so results are ignored.
    sqlite3_exec(db, createTableSQL, nullptr, nullptr, nullptr);

    // Closes the database connection, freeing resources.
    sqlite3_close(db);
}
