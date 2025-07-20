// Core.cpp : Defines the functions for the static library.
//
// This is a standard file header comment. It describes the file's purpose.

#include "pch.h"           // Precompiled header, speeds up compilation.
#include "framework.h"     // Project-specific header, may include common definitions.
#include <sqlite3.h>       // SQLite C API header, needed for database operations.

// This is a library function
void fnCore()
{
    sqlite3* db;
    sqlite3_open("FitApp.db", &db);

    // Drop tables first to remove old schema and data
    sqlite3_exec(db, "DROP TABLE IF EXISTS userstats;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "DROP TABLE IF EXISTS users;", nullptr, nullptr, nullptr);

    // Now create tables with the new schema
    const char* createUsersTableSQL =
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY, "
        "password TEXT NOT NULL, "
        "salt TEXT NOT NULL"
        ");";

    const char* createUserStatsTableSQL =
        "CREATE TABLE IF NOT EXISTS userstats ("
        "User TEXT PRIMARY KEY, "
        "Height INTEGER, "
        "Weight INTEGER, "
        "FOREIGN KEY(User) REFERENCES users(username)"
        ");";

    sqlite3_exec(db, createUsersTableSQL, nullptr, nullptr, nullptr);
    sqlite3_exec(db, createUserStatsTableSQL, nullptr, nullptr, nullptr);

    sqlite3_close(db);
}
