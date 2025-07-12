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
		User(const std::string& userName, const std::string& email, const std::string& password)
			: name(userName), email(email), password(password) {}
		const std::string& getName() const { return name; }
		const std::string& getEmail() const { return email; }
		const std::string& getPassword() const { return password; }
		void addWeightEntry(const weightEntry& entry) {
			weightEntries.push_back(entry);
		}
		const std::vector<weightEntry>& getWeightEntries() const {
			return weightEntries;
		}
	private:
		std::string name;
		std::string email;
		std::string password;
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

		sqlite3_bind_text(stmt, 1, user.getName().c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, user.getEmail().c_str