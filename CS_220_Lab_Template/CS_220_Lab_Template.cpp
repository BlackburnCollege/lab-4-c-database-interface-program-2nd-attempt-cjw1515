#include <iostream>
#include "sqlite3.h"
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
using namespace std;

class PokemonDB {
public:
	PokemonDB() : db(nullptr) {}

	bool open(const std::string& path) {
		return sqlite3_open(path.c_str(), &db) == SQLITE_OK;

	}

	void close() {
		if (db) {
			sqlite3_close(db);
			db = nullptr;
		}
	}
sqlite3* getDB() {
		return db;
}
private:
	sqlite3* db;
	};

	static int callback(void* data, int argc, char** argv, char** azColName)
	{
		int i;

		for (i = 0; i < argc; i++)
		{
			cout << azColName[i] << " = ";
			if (argv[i] == NULL)
			{
				cout << "NULL\n;";
			}
			else
			{
				cout << argv[i] << '\n';;
			}
		}

		cout << '\n';
		return 0;
	}

	void createTables(sqlite3* db)
	{
		char* errMsg = nullptr;
		const char* sql =
			"CREATE TABLE Sets("
			"Set_ID INTEGER PRIMARY KEY,"
			"Generation INTEGER,"
			"Set_Name TEXT,"
			"Total_Cards INTEGER"
			");"

			"CREATE TABLE Types("
			"Type_ID INTEGER PRIMARY KEY,"
			"Type_name TEXT"
			");"

			"CREATE TABLE Rarity("
			"Rarity_ID INTEGER PRIMARY KEY,"
			"Rarity_name TEXT"
			");"

			"CREATE TABLE Cards("
			"Card_ID INTEGER PRIMARY KEY,"
			"Name TEXT,"
			"Type_ID INTEGER,"
			"Rarity_ID INTEGER,"
			"Card_number TEXT,"
			"Set_ID INTEGER,"
			"FOREIGN KEY (Set_ID) REFERENCES Sets(Set_ID),"
			"FOREIGN KEY (Type_ID) REFERENCES Types(Type_ID),"
			"FOREIGN KEY (Rarity_ID) REFERENCES Rarity(Rarity_ID)"
			");"

			"CREATE TABLE Ownership("
			"Ownership_ID INTEGER PRIMARY KEY,"
			"Card_ID INTEGER,"
			"Quantity INTEGER,"
			"Method_of_obtaining TEXT,"
			"Purchase_price REAL,"
			"Pack_price REAL,"
			"Market_price REAL,"
			"Date_of_price_evaluation TEXT,"
			"FOREIGN KEY (Card_ID) REFERENCES Cards(Card_ID)"
			");";

		if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
			cout << "Error creating tables: " << errMsg << endl;
			sqlite3_free(errMsg);
		}
		else {
			cout << "Tables created successfully." << endl;
		}
	}

	void importCSV(sqlite3* db, const string& filename, const string& table, int columns)
	{
		ifstream file(filename);
		string line;
		if (!file.is_open()) {
			cout << "Could not open file: " << filename << endl;
			return;
		}
		while (getline(file, line)) {
			vector<string> values;
			string value;
			stringstream ss(line);

			while (getline(ss, value, ',')) {
				values.push_back(value);
			}
			string sql = "INSERT INTO " + table + " VALUES(";
			for (int i = 0; i < columns; i++) {
				sql += "'" + values[i] + "'";
				if (i < columns - 1) sql += ",";
			}
			sql += ");";
			sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
		}
		cout << "Imported data into " << table << " from " << filename << endl;
	}

	void importData(sqlite3* db)
	{
		cout << "Importing data from CSV files." << endl;
		importCSV(db, "sets.csv", "Sets", 4);
		importCSV(db, "types.csv", "Types", 2);
		importCSV(db, "rarity.csv", "Rarity", 2);
		importCSV(db, "cards.csv", "Cards", 6);
		importCSV(db, "ownership.csv", "Ownership", 8);

		cout << "CSV imported" << endl;
	}

	void simpleSELECT(sqlite3* db)
	{
		cout << "\n-Simple Select: All Cards-\n";
		const char* sql = "SELECT * FROM Cards;";
		char* errMsg = nullptr;
		int rc = sqlite3_exec(db, sql, callback, nullptr, &errMsg);

		if (rc != SQLITE_OK)
		{
			cout << "Error: " << errMsg << endl;
			sqlite3_free(errMsg);
		}
	}

	void complexSelect(sqlite3* db)
	{
		cout << "\n-Complex Select: Cards with Type and Rarity-\n";
		const char* sql =
			"SELECT Cards.Card_ID, Cards.Name, Types.Type_name, Rarity.Rarity_name "
			"FROM Cards "
			"LEFT JOIN Types ON Cards.Type_ID = Types.Type_ID "
			"LEFT JOIN Rarity ON Cards.Rarity_ID = Rarity.Rarity_ID "
			"ORDER BY Cards.Card_ID;";

		char* errMsg = nullptr;
		int rc = sqlite3_exec(db, sql, callback, nullptr, &errMsg);
		if (rc != SQLITE_OK) {
			cout << "Error: " << errMsg << endl;
			sqlite3_free(errMsg);
		}
	}

	void userSelect(sqlite3* db)
	{
		cin.ignore();
		cout << "\n-User Defined Select-\n";
		cout << "Enter Select Statement:\n";

		string query;
		getline(cin, query);
		char* errMsg = nullptr;
		int rc = sqlite3_exec(db, query.c_str(), callback, nullptr, &errMsg);
		if (rc != SQLITE_OK) {
			cout << "Error: " << errMsg << endl;
			sqlite3_free(errMsg);
		}
	}


int main() {
	PokemonDB pokemonDB;
	if (!pokemonDB.open("pokemon_collection.db")) {
		cout << "Could not open DB.\n";
		return 1;
	}
	
	int choice = 0;
	
	while (true) {
		cout << "\n-MENU-\n";
		cout << "1) Create Database\n";
		cout << "2) Import Data\n";
		cout << "3) Simple SELECT\n";
		cout << "4) Complex SELECT\n";
		cout << "5) User-defined SELECT\n";
		cout << "6) Exit\n";
		cout << "Enter your choice: ";
		
		cin >> choice;

		if (choice == 1) {
			createTables(pokemonDB.getDB());
		}
		else if (choice == 2) {
			importData(pokemonDB.getDB());
		}
		else if (choice == 3) {
			simpleSELECT(pokemonDB.getDB());
		}
		else if (choice == 4) {
			complexSelect(pokemonDB.getDB());
		}
		else if (choice == 5) {
			userSelect(pokemonDB.getDB());
		}
		else if (choice == 6) {
			pokemonDB.close();
			break;
		}
		else {
			cout << "Invalid selection.\n";
		}
	}
	sqlite3_close(pokemonDB.getDB());
	return 0; 
}