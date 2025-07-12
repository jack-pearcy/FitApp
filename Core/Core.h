#pragma once

#include <string>
#include <vector>
#include <ctime>

namespace Core {

	struct weightEntry {
		double weightLbs;
		std::time_t dateTimeStamp; // Using std::time_t for date representation
	};

    class User {
    public:
        User(const std::wstring& userName,
            const std::string& pwdHash,
            const std::string& salt);

        const std::string& getUserName() const { return userName; }
        const std::string& getPassword() const { return password; }
        void addWeightEntry(const weightEntry& entry) {
            weightEntries.push_back(entry);
        }
        const std::vector<weightEntry>& getWeightEntries() const {
            return weightEntries;
        }

    private:
        std::string userName;
        std::string password;
        std::string salt;
        std::vector<weightEntry> weightEntries;
    };






}