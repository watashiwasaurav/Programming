#include <iostream>
#include <mysql.h>
#include <string>
#include <sstream>
#include <cstdlib>

// Declare MySQL connection globally
MYSQL* connection;

// Function prototypes
void createTables();
void registerUser();
bool loginUser();
void storeCustomerDetails(const std::string& username);
void displayUserInfo(const std::string& username);

int main() {
    // Establish MySQL database connection
    connection = mysql_init(NULL);
    if (connection == NULL) {
        std::cerr << "Error: Failed to initialize MySQL connection." << std::endl;
        return 1;
    }

    const char* host = "localhost";
    const char* user = "root";
    const char* password = "root";
    const char* database = "fitness_world";
    unsigned int port = 3306; // Default MySQL port
    if (!mysql_real_connect(connection, host, user, password, database, port, NULL, 0)) {
        std::cerr << "Error: Failed to connect to MySQL database." << std::endl;
        std::cerr << "Error message: " << mysql_error(connection) << std::endl;
        mysql_close(connection);
        return 1;
    }

    std::cout << "Connected to MySQL database successfully." << std::endl;

    // Check if the email column exists in the users table
    MYSQL_RES* res = mysql_list_fields(connection, "users", NULL);
    if (res == NULL) {
        std::cerr << "Error: Failed to get table information." << std::endl;
        std::cerr << "Error message: " << mysql_error(connection) << std::endl;
        mysql_close(connection);
        return 1;
    }
    MYSQL_FIELD* field;
    bool emailColumnExists = false;
    while ((field = mysql_fetch_field(res))) {
        if (std::string(field->name) == "email") {
            emailColumnExists = true;
            break;
        }
    }
    mysql_free_result(res);

    // If email column doesn't exist, prompt user to recreate the table with email column
    if (!emailColumnExists) {
        std::cerr << "Error: The 'email' column is missing in the 'users' table." << std::endl;
        std::cerr << "Please recreate the 'users' table with the 'email' column." << std::endl;
        mysql_close(connection);
        return 1;
    }

    // Create tables if they don't exist
    createTables();

    // Prompt user for login or registration
    std::cout << "Select an option:" << std::endl;
    std::cout << "1. Register" << std::endl;
    std::cout << "2. Login" << std::endl;
    std::cout << "Your choice: ";
    int choice;
    std::cin >> choice;

    switch (choice) {
        case 1:
            registerUser();
            break;
        case 2:
            if (!loginUser()) {
                std::cerr << "Error: Login failed." << std::endl;
            }
            break;
        default:
            std::cerr << "Error: Invalid choice." << std::endl;
            break;
    }

    // Close MySQL database connection
    mysql_close(connection);

    return 0;
}

void createTables() {
    std::string query = "CREATE TABLE IF NOT EXISTS users ("
                        "id INT AUTO_INCREMENT PRIMARY KEY,"
                        "username VARCHAR(50) NOT NULL UNIQUE,"
                        "password VARCHAR(100) NOT NULL,"
                        "email VARCHAR(100) NOT NULL UNIQUE"
                        ")";
    if (mysql_query(connection, query.c_str()) != 0) {
        std::cerr << "Error: Failed to create users table." << std::endl;
        std::cerr << "Error message: " << mysql_error(connection) << std::endl;
    }

    query = "CREATE TABLE IF NOT EXISTS customer_details ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "username VARCHAR(50) NOT NULL UNIQUE,"
            "training_plan VARCHAR(50) NOT NULL,"
            "current_weight DOUBLE NOT NULL,"
            "target_weight_category VARCHAR(50) NOT NULL,"
            "sauna_option INT NOT NULL,"
            "num_sessions INT NOT NULL,"
            "num_hours DOUBLE NOT NULL"
            ")";
    if (mysql_query(connection, query.c_str()) != 0) {
        std::cerr << "Error: Failed to create customer_details table." << std::endl;
        std::cerr << "Error message: " << mysql_error(connection) << std::endl;
    }
}

void registerUser() {
    std::string username, password, email;

    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;
    std::cout << "Enter email: ";
    std::cin >> email;

    std::string query = "INSERT INTO users (username, password, email) VALUES ('" + username + "', '" + password + "', '" + email + "')";
    if (mysql_query(connection, query.c_str()) != 0) {
        std::cerr << "Error: Failed to register user." << std::endl;
        std::cerr << "Error message: " << mysql_error(connection) << std::endl;
        return;
    }

    std::cout << "User registered successfully." << std::endl;

    // Store customer details in customer_details table
    storeCustomerDetails(username);
}

bool loginUser() {
    std::string username, password;

    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;

    std::string query = "SELECT * FROM users WHERE username='" + username + "' AND password='" + password + "'";
    if (mysql_query(connection, query.c_str()) != 0) {
        std::cerr << "Error: Failed to execute query." << std::endl;
        std::cerr << "Error message: " << mysql_error(connection) << std::endl;
        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection);
    if (result == NULL) {
        std::cerr << "Error: No result set returned." << std::endl;
        return false;
    }

    int num_rows = mysql_num_rows(result);
    mysql_free_result(result);

    if (num_rows == 1) {
        std::cout << "Login successful." << std::endl;
        // Display user information
        displayUserInfo(username);
        return true;
    } else {
        std::cerr << "Error: Invalid username or password." << std::endl;
        return false;
    }
}

void storeCustomerDetails(const std::string& username) {
    std::string trainingPlan, targetWeightCategory;
    double currentWeight, numHours;
    int saunaOption, numSessions;

    std::cout << "Enter Training Plan (Beginner/Intermediate/Elite): ";
    std::cin >> trainingPlan;
    std::cout << "Enter Current Weight (kg): ";
    std::cin >> currentWeight;
    std::cout << "Enter Target Weight Category: ";
    std::cin >> targetWeightCategory;
    std::cout << "Sauna Option (1 for Yes, 0 for No): ";
    std::cin >> saunaOption;
    std::cout << "Enter Number of Swimming Sessions: ";
    std::cin >> numSessions;
    std::cout << "Enter Number of Private Coaching Hours: ";
    std::cin >> numHours;

    // Insert the customer details into the database
    std::stringstream query;
    query << "INSERT INTO customer_details (username, training_plan, current_weight, target_weight_category, sauna_option, num_sessions, num_hours) VALUES ('" << username << "', '" << trainingPlan << "', " << currentWeight << ", '" << targetWeightCategory << "', " << saunaOption << ", " << numSessions << ", " << numHours << ")";

    if (mysql_query(connection, query.str().c_str()) != 0) {
        std::cerr << "Error: Failed to store customer details." << std::endl;
        std::cerr << "Error message: " << mysql_error(connection) << std::endl;
        return;
    }

    std::cout << "Customer details stored successfully." << std::endl;
}

void displayUserInfo(const std::string& username) {
    // Retrieve user information from the database
    std::string query = "SELECT * FROM customer_details WHERE username='" + username + "'";
    if (mysql_query(connection, query.c_str()) != 0) {
        std::cerr << "Error: Failed to execute query." << std::endl;
        std::cerr << "Error message: " << mysql_error(connection) << std::endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(connection);
    if (result == NULL) {
        std::cerr << "Error: No result set returned." << std::endl;
        return;
    }

    MYSQL_ROW row;
    if ((row = mysql_fetch_row(result))) {
        std::string trainingPlan = row[2];
        double currentWeight = atof(row[3]);
        std::string targetWeightCategory = row[4];
        int saunaOption = atoi(row[5]);
        int numSessions = atoi(row[6]);
        double numHours = atof(row[7]);

        // Calculate costs
        double trainingFee = 0.0;
        if (trainingPlan == "Beginner") {
            trainingFee = 1000 * numSessions;
        } else if (trainingPlan == "Intermediate") {
            trainingFee = 2000 * numSessions;
        } else if (trainingPlan == "Elite") {
            trainingFee = 3000 * numSessions;
        }

        double saunaCost = 0.0;
        if (saunaOption == 1) {
            saunaCost = 1500;
        }

        double swimmingCost = 500 * numSessions;
        double privateTrainerCost = 500 * numHours;

        double totalCost = trainingFee + saunaCost + swimmingCost + privateTrainerCost;

        // Output user information
        system("cls");
        std::cout << "\t\t\tCustomer Information:" << std::endl;
        std::cout << "\t\t\t--------------------" << std::endl;
        std::cout << "\t\t\tCustomer Name:\t\t\t" << username << std::endl;
        std::cout << "\t\t\tTraining Plan:\t\t\t" << trainingPlan << std::endl;
        std::cout << "\t\t\tCurrent Weight (kg):\t\t" << currentWeight << std::endl;
        std::cout << "\t\t\tTarget Weight Category:\t\t" << targetWeightCategory << std::endl;
        std::cout << "\t\t\tSauna Option:\t\t\t" << (saunaOption == 1 ? "Yes" : "No") << std::endl;
        std::cout << "\t\t\tNumber of Swimming Sessions:\t" << numSessions << std::endl;
        std::cout << "\t\t\tNumber of Private Coaching Hours:\t" << numHours << std::endl;
        std::cout << "\t\t\tItemized List of Costs for the Month:" << std::endl;
        std::cout << "\t\t\t------------------------------------" << std::endl;
        std::cout << "\t\t\t- Training Fee:\t\t\t" << trainingFee << std::endl;
        std::cout << "\t\t\t- Sauna Cost:\t\t\t" << saunaCost << std::endl;
        std::cout << "\t\t\t- Swimming Cost:\t\t" << swimmingCost << std::endl;
        std::cout << "\t\t\t- Private Trainer Cost:\t\t" << privateTrainerCost << std::endl;
        std::cout << "\t\t\t------------------------------------" << std::endl;
        std::cout << "\t\t\tTotal Cost for the Month:\t\t" << totalCost << std::endl;
    }

    mysql_free_result(result);
}

