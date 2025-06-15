#include <iostream>
#include <vector>
using namespace std;

const string ADMIN_PASSWORD = "mahmoud";
const int MAX_ACCOUNTS = 100;

class Account {
protected:
    int balance;
    int accountID;
public:
    Account(int id = 0, int b = 0) : accountID(id), balance(b) {}
    virtual void deposit(int amount) = 0;
    virtual bool withdraw(int amount) = 0;
    virtual void showBalance() const = 0;
    virtual string getType() const = 0;
    virtual ~Account() {}
    int getBalance() const { 
        return balance; }
    int getAccountID() const {
         return accountID; }
};

class CheckingAccount : public Account {
public:
    CheckingAccount(int id = 0, int b = 0) : Account(id, b) {}
    void deposit(int amount) override { balance += amount; }
    bool withdraw(int amount) override {
        if (amount > balance) return false;
        balance -= amount;
        return true;
    }
    void showBalance() const override {
        cout << "Checking Balance " << balance << "\n";
    }
    string getType() const override {
         return "Checking"; }
};

class SavingsAccount : public Account {
public:
    SavingsAccount(int id = 0, int b = 0) : Account(id, b) {}
    void deposit(int amount) override { balance += amount; }
    bool withdraw(int) override { return false; }
    void showBalance() const override {
        cout << "Savings Balance " << balance << "\n";
    }
    string getType() const override { return "Savings"; }
};

class Transaction {
public:
    virtual void showBalances() = 0;
    virtual void depositToChecking(int) = 0;
    virtual void depositToSavings(int) = 0;
    virtual void withdrawFromChecking(int) = 0;
    virtual void transferToSavings(int) = 0;
    virtual void changePIN() = 0;
    virtual ~Transaction() {}
};

class Customer : public Transaction {
    string userID;
    int PIN;
    CheckingAccount checking;
    SavingsAccount savings;
public:
    Customer(string id, int p, int cBal = 0, int sBal = 0)
        : userID(id), PIN(p), checking(1, cBal), savings(2, sBal) {}

    string getUserID() const { return userID; }

    bool authenticate() {
        int attempts = 0, input;
        while (attempts < 3) {
            cout << "Enter your PIN: ";
            cin >> input;
            if (input == PIN) return true;
            cout << "Incorrect PIN.\n";
            attempts++;
        }
        cout << "Too many failed attempts.\n";
        return false;
    }

    void showBalances() override {
        checking.showBalance();
        savings.showBalance();
    }

    void depositToChecking(int amount) override { checking.deposit(amount); }
    void depositToSavings(int amount) override { savings.deposit(amount); }
    void withdrawFromChecking(int amount) override { checking.withdraw(amount); }
    void transferToSavings(int amount) override {
        if (checking.withdraw(amount)) savings.deposit(amount);
    }
    void changePIN() override {
        int oldPIN, newPIN;
        cout << "Enter old PIN: "; cin >> oldPIN;
        if (oldPIN == PIN) {
            cout << "Enter new PIN: "; cin >> newPIN;
            PIN = newPIN;
            cout << "PIN changed.\n";
        } else {
            cout << "Incorrect old PIN.\n";
        }
    }
};

class ATM {
    vector<Customer> accounts;

    Customer* findCustomer(const string& id) {
        for (auto& c : accounts) if (c.getUserID() == id) return &c;
        return nullptr;
    }

public:
    ATM() {
        accounts.emplace_back("user1111", 1234, 1000, 500);
        cout << "Default customer loaded.\n";
    }

    void addAccount() {
        if (accounts.size() >= MAX_ACCOUNTS) return;
        string id; int pin;
        cout << "New User ID: "; cin >> id;
        cout << "PIN: "; cin >> pin;
        if (findCustomer(id)) { cout << "User exists.\n"; return; }
        accounts.emplace_back(id, pin);
        cout << "Account added.\n";
    }

    void deleteAccount() {
        string id;
        cout << "Delete User ID: ";
        cin >> id;

        bool found = false;
        for (size_t i = 0; i < accounts.size(); ++i) {
            if (accounts[i].getUserID() == id) {
                accounts.erase(accounts.begin() + i);
                cout << "Deleted.";
                found = true;
                break;
            }
        }
        if (!found) {
            cout << "Not found.";
        }
    }

    void showAllAccounts() {
        for (auto& c : accounts) {
            cout << "User: " << c.getUserID() << "\n";
            c.showBalances();
        }
    }

    void customerMenu(Customer& c) {
        int ch, amt;
        do {
            cout << "1-Deposit(C) 2-Withdraw(C) 3-Deposit(S) 4-Balances 5-Change PIN 6-Transfer(S) 0-Logout\nChoice: ";
            cin >> ch;
            switch (ch) {
                case 1: cout << "Amt: "; cin >> amt; c.depositToChecking(amt); break;
                case 2: cout << "Amt: "; cin >> amt; c.withdrawFromChecking(amt); break;
                case 3: cout << "Amt: "; cin >> amt; c.depositToSavings(amt); break;
                case 4: c.showBalances(); break;
                case 5: c.changePIN(); break;
                case 6: cout << "Amt: "; cin >> amt; c.transferToSavings(amt); break;
            }
        } while (ch != 0);
    }

    void customerLogin() {
        string id;
        cout << "User ID: "; cin >> id;
        Customer* c = findCustomer(id);
        if (c && c->authenticate()) customerMenu(*c);
    }

    void adminMenu() {
        int ch;
        do {
            cout << "1-Add 2-Delete 3-View All 0-Exit\nChoice: ";
            cin >> ch;
            switch (ch) {
                case 1: addAccount();
                 break;
                case 2: deleteAccount();
                 break;
                case 3: showAllAccounts();
                 break;
            }
        } while (ch != 0);
    }
};

int main() {
    ATM atm;
    int ch, tries = 0;
    do {
        cout << "1-Admin 2-Customer 0-Exit\nChoice: "; cin >> ch;
        if (ch == 1) {
            string pass;
            for (int i = 0; i < 3; ++i) {
                cout << "Password: "; cin >> pass;
                if (pass == ADMIN_PASSWORD) { atm.adminMenu(); break; }
                else cout << "Wrong.\n";
            }
        } else if (ch == 2) atm.customerLogin();
    } while (ch != 0);
    return 0;
}
