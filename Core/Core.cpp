// Core.cpp : Defines the functions for the static library.
//
// This is a standard file header comment. It describes the file's purpose.

#include "pch.h"           // Precompiled header, speeds up compilation.
#include "framework.h"     // Project-specific header, may include common definitions.
#include <sqlite3.h>       // SQLite C API header, needed for database operations.

// This is a library function
void fnCore()
{
    // Create 'users' table
    const char* createUsersTableSQL =
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY, "
        "password TEXT NOT NULL, "
        "salt TEXT NOT NULL"
        ");";

    // Create 'userstats' table
    const char* createUserStatsTableSQL =
        "CREATE TABLE IF NOT EXISTS userstats ("
        "username TEXT PRIMARY KEY, "
        "height_in INTEGER, "
        // Add more stats fields as needed
        "FOREIGN KEY(username) REFERENCES users(username)"
        ");";

    sqlite3* db;
    sqlite3_open("FitApp.db", &db);

    sqlite3_exec(db, createUsersTableSQL, nullptr, nullptr, nullptr);
    sqlite3_exec(db, createUserStatsTableSQL, nullptr, nullptr, nullptr);

    sqlite3_close(db);
}
