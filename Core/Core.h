#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <sqlite3.h>
#include <locale>
#include <codecvt>

void fnCore();

namespace Core {


	struct weightEntry {
		double weightLbs;
		std::time_t dateTimeStamp; // Using std::time_t for date representation
	};

	class User {
	public:
		// Constructor matching the argument list
		User(const std::wstring& userName, const std::string& passwordHash, const std::string& salt) 
			: userName(userName), passwordHash(passwordHash), salt(salt) {}

		const std::wstring& getName() const { return userName; }
		const std::string& getPasswordHash() const { return passwordHash; }
		const std::string& getSalt() const { return salt; }
		void addWeightEntry(const weightEntry& entry) {
			weightEntries.push_back(entry);
		}
		const std::vector<weightEntry>& getWeightEntries() const {
			return weightEntries;
		}
	private:
		std::wstring userName;
		std::string passwordHash;
		std::string salt;
		std::vector<weightEntry> weightEntries;
	};

	// Function to insert a user into the SQLite database
	inline bool InsertUserToDatabase(const User& user) {
		sqlite3* db;
		int rc = sqlite3_open("FitApp.db", &db);
		if (rc != SQLITE_OK) {
			sqlite3_close(db);
			return false;
		}

		const char* sql = "INSERT INTO Users (Username, Password, Salt) VALUES (?, ?, ?);";
		sqlite3_stmt* stmt;
		rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK) {
			sqlite3_close(db);
			return false;
		}

		// Convert wstring to UTF-8 string for database
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		std::string usernameUtf8 = conv.to_bytes(user.getName());

		sqlite3_bind_text(stmt, 1, usernameUtf8.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, user.getPasswordHash().c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 3, user.getSalt().c_str(), -1, SQLITE_TRANSIENT);

		rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		
		return (rc == SQLITE_DONE);
	}
}