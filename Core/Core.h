#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <sqlite3.h>

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
		int rc = sqlite3_open("fitapp.db", &db);
		if (rc != SQLITE_OK) {
			sqlite3_close(db);
			return false;
		}

		const char* sql = "INSERT INTO users (username, email, password) VALUES (?, ?, ?);";
		sqlite3_stmt* stmt;
		rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK) {
			sqlite3_close(db);
			return false;
		}

		sqlite3_bind_text(stmt, 1, reinterpret_cast<const char*>(user.getName().c_str()), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 3, user.getPasswordHash().c_str(), -1, SQLITE_TRANSIENT);
	}
}