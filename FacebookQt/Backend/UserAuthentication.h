#pragma once
#include <iostream>
#include <string>
#include <regex>
#include <cctype>
#include "User.h"
using namespace std;

class UserAuthentication {
private:
    User* head;

    static bool isBlank(const string& value) {
        for (size_t i = 0; i < value.length(); i++) {
            if (!isspace(static_cast<unsigned char>(value[i]))) return false;
        }
        return true;
    }

public:
    UserAuthentication() : head(nullptr) {}

    User* GetHead() const {
        return head;
    }

    User* findUserByID(int userID) const {
        User* temp = head;
        while (temp && temp->userID != userID) {
            temp = temp->next;
        }
        return temp;
    }

    User* findUserByEmail(const string& email) const {
        User* temp = head;
        while (temp && temp->email != email) {
            temp = temp->next;
        }
        return temp;
    }

    static bool isValidName(const string& name) {
        if (name.empty() || isBlank(name)) return false;
        for (size_t i = 0; i < name.length(); i++) {
            unsigned char ch = static_cast<unsigned char>(name[i]);
            if (!isalpha(ch) && !isspace(ch)) return false;
        }
        return true;
    }

    static bool isValidEmail(const string& email) {
        if (email.empty() || isBlank(email)) return false;
        const regex pattern("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
        return regex_match(email, pattern);
    }

    static bool isValidPassword(const string& password) {
        if (password.length() < 8) return false;

        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (size_t i = 0; i < password.length(); i++) {
            unsigned char ch = static_cast<unsigned char>(password[i]);
            if (isupper(ch)) hasUpper = true;
            else if (islower(ch)) hasLower = true;
            else if (isdigit(ch)) hasDigit = true;
            else hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    static bool isValidGender(const string& gender) {
        return gender == "Male" || gender == "Female" || gender == "Other";
    }

    bool validateSignUp(int id, const string& name, const string& email,
        const string& password, const string& gender, int age) const {
        if (id <= 0) {
            cout << "User ID must be greater than zero.\n";
            return false;
        }
        if (findUserByID(id)) {
            cout << "User ID already exists.\n";
            return false;
        }
        if (!isValidName(name)) {
            cout << "Name must contain only alphabetic characters and spaces.\n";
            return false;
        }
        if (!isValidEmail(email)) {
            cout << "Please enter a valid email address.\n";
            return false;
        }
        if (findUserByEmail(email)) {
            cout << "Email already exists. Try login.\n";
            return false;
        }
        if (password.empty() || isBlank(password)) {
            cout << "Password cannot be empty.\n";
            return false;
        }
        if (!isValidPassword(password)) {
            cout << "Password must be at least 8 characters and include uppercase, lowercase, digit, and special character.\n";
            return false;
        }
        if (!isValidGender(gender)) {
            cout << "Gender must be Male, Female, or Other.\n";
            return false;
        }
        if (age < 13 || age > 120) {
            cout << "Age must be between 13 and 120.\n";
            return false;
        }
        return true;
    }

    bool addUser(User* newUser) {
        if (!newUser || findUserByID(newUser->userID) || findUserByEmail(newUser->email)) {
            return false;
        }

        newUser->next = nullptr;
        if (!head) {
            head = newUser;
        }
        else {
            User* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newUser;
        }
        return true;
    }

    bool signUp(int id, const string& name, const string& email, const string& password,
        const string& gender, int age, bool isPublic) {
        if (!validateSignUp(id, name, email, password, gender, age)) return false;

        User* newUser = new User(id, name, email, password, gender, age, isPublic);
        addUser(newUser);

        cout << "User registered successfully!\n";
        return true;
    }

    bool login(int id, const string& password) const {
        if (id <= 0) {
            cout << "User ID must be greater than zero.\n";
            return false;
        }
        if (password.empty() || isBlank(password)) {
            cout << "Password cannot be empty.\n";
            return false;
        }

        User* user = findUserByID(id);
        if (!user) {
            cout << "No user found with this ID.\n";
            return false;
        }

        if (user->password != password) {
            cout << "Incorrect password.\n";
            return false;
        }

        cout << "Login successful! Welcome, " << user->name << "!\n";
        return true;
    }

    bool login(const string& email, const string& password) const {
        if (!isValidEmail(email)) {
            cout << "Please enter a valid email address.\n";
            return false;
        }
        if (password.empty() || isBlank(password)) {
            cout << "Password cannot be empty.\n";
            return false;
        }

        User* user = findUserByEmail(email);
        if (!user) {
            cout << "No user found with this email.\n";
            return false;
        }

        if (user->password != password) {
            cout << "Incorrect password.\n";
            return false;
        }

        cout << "Login successful! Welcome, " << user->name << "!\n";
        return true;
    }

    void clear() {
        while (head) {
            User* temp = head;
            head = head->next;
            delete temp;
        }
    }

    ~UserAuthentication() {
        clear();
    }
};
