#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QString>
#include <vector>
#include <algorithm>
#include <iostream>
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
    virtual QString showBalance() const = 0;
    virtual QString getType() const = 0;
    virtual ~Account() {}
    int getBalance() const { return balance; }
    int getAccountID() const { return accountID; }
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
    QString showBalance() const override {
        return QString("Checking Balance: $%1").arg(balance);
    }
    QString getType() const override { return "Checking"; }
};

class SavingsAccount : public Account {
public:
    SavingsAccount(int id = 0, int b = 0) : Account(id, b) {}
    void deposit(int amount) override { balance += amount; }
    bool withdraw(int) override { return false; }
    QString showBalance() const override {
        return QString("Savings Balance: $%1").arg(balance);
    }
    QString getType() const override { return "Savings"; }
};

class Customer {
    QString userID;
    int PIN;
    CheckingAccount checking;
    SavingsAccount savings;
public:
    Customer(QString id = "", int p = 0, int cBal = 0, int sBal = 0)
        : userID(id), PIN(p), checking(1, cBal), savings(2, sBal) {}

    QString getUserID() const { return userID; }

    bool authenticate(int inputPIN) {
        return inputPIN == PIN;
    }

    QString showBalances() {
        return checking.showBalance() + "\n" + savings.showBalance();
    }

    void depositToChecking(int amt) { checking.deposit(amt); }
    void depositToSavings(int amt) { savings.deposit(amt); }
    void withdrawFromChecking(int amt) { checking.withdraw(amt); }
    void transferToSavings(int amt) {
        if (checking.withdraw(amt)) savings.deposit(amt);
    }

    bool changePIN(int oldPIN, int newPIN) {
        if (oldPIN == PIN) {
            PIN = newPIN;
            return true;
        }
        return false;
    }
};

class ATMApp : public QWidget {
    Q_OBJECT
    vector<Customer> accounts;

public:
    ATMApp(QWidget *parent = nullptr) : QWidget(parent) {
        QPushButton *adminBtn = new QPushButton("Admin Login");
        QPushButton *customerBtn = new QPushButton("Customer Login");

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(adminBtn);
        layout->addWidget(customerBtn);

        connect(adminBtn, &QPushButton::clicked, this, &ATMApp::adminLogin);
        connect(customerBtn, &QPushButton::clicked, this, &ATMApp::customerLogin);

        accounts.emplace_back("user1111", 1234, 1000, 500);
    }

private slots:
    void adminLogin() {
        bool ok;
        QString password = QInputDialog::getText(this, "Admin Login", "Enter Password:", QLineEdit::Password, "", &ok);
        if (!ok || password.toStdString() != ADMIN_PASSWORD) {
            QMessageBox::warning(this, "Login Failed", "Incorrect password.");
            return;
        }

        while (true) {
            QStringList actions = {"Add Account", "Delete Account", "View All", "Exit"};
            QString choice = QInputDialog::getItem(this, "Admin Menu", "Select an action:", actions, 0, false, &ok);
            if (!ok || choice == "Exit") break;

            if (choice == "Add Account") {
                QString id = QInputDialog::getText(this, "Add", "User ID:");
                int pin = QInputDialog::getInt(this, "Add", "PIN:");
                if (findCustomer(id)) {
                    QMessageBox::warning(this, "Error", "User already exists.");
                } else {
                    accounts.emplace_back(id, pin);
                    QMessageBox::information(this, "Success", "Account created.");
                }
            } else if (choice == "Delete Account") {
                QString id = QInputDialog::getText(this, "Delete", "User ID:");
                auto it = std::remove_if(accounts.begin(), accounts.end(), [&](Customer &c) {
                    return c.getUserID() == id;
                });
                if (it != accounts.end()) {
                    accounts.erase(it, accounts.end());
                    QMessageBox::information(this, "Success", "Account deleted.");
                } else {
                    QMessageBox::warning(this, "Error", "Account not found.");
                }
            } else if (choice == "View All") {
                QString output;
                for (auto &c : accounts) {
                    output += "User: " + c.getUserID() + "\n" + c.showBalances() + "\n\n";
                }
                QMessageBox::information(this, "Accounts", output);
            }
        }
    }

    void customerLogin() {
        bool ok;
        QString id = QInputDialog::getText(this, "Customer Login", "Enter User ID:");
        Customer *cust = findCustomer(id);
        if (!cust) {
            QMessageBox::warning(this, "Error", "User not found.");
            return;
        }

        int pin = QInputDialog::getInt(this, "PIN", "Enter PIN:", 0, 0, 9999, 1, &ok);
        if (!ok || !cust->authenticate(pin)) {
            QMessageBox::warning(this, "Login Failed", "Incorrect PIN.");
            return;
        }

        while (true) {
            QStringList actions = {
                "Deposit to Checking", "Withdraw from Checking", "Deposit to Savings",
                "Transfer to Savings", "Show Balances", "Change PIN", "Logout"
            };
            QString choice = QInputDialog::getItem(this, "Customer Menu", "Select:", actions, 0, false, &ok);
            if (!ok || choice == "Logout") break;

            int amt;
            if (choice == "Deposit to Checking") {
                amt = QInputDialog::getInt(this, "Amount", "Deposit Amount:");
                cust->depositToChecking(amt);
            } else if (choice == "Withdraw from Checking") {
                amt = QInputDialog::getInt(this, "Amount", "Withdraw Amount:");
                cust->withdrawFromChecking(amt);
            } else if (choice == "Deposit to Savings") {
                amt = QInputDialog::getInt(this, "Amount", "Deposit Amount:");
                cust->depositToSavings(amt);
            } else if (choice == "Transfer to Savings") {
                amt = QInputDialog::getInt(this, "Amount", "Transfer Amount:");
                cust->transferToSavings(amt);
            } else if (choice == "Show Balances") {
                QMessageBox::information(this, "Balances", cust->showBalances());
            } else if (choice == "Change PIN") {
                int oldPIN = QInputDialog::getInt(this, "Change PIN", "Enter Old PIN:");
                int newPIN = QInputDialog::getInt(this, "Change PIN", "Enter New PIN:");
                if (cust->changePIN(oldPIN, newPIN))
                    QMessageBox::information(this, "Success", "PIN changed.");
                else
                    QMessageBox::warning(this, "Error", "Wrong old PIN.");
            }
        }
    }

private:
    Customer* findCustomer(const QString &id) {
        for (auto &c : accounts)
            if (c.getUserID() == id)
                return &c;
        return nullptr;
    }
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ATMApp window;
    window.setWindowTitle("ATM System");
    window.resize(300, 200);
    window.show();
    return app.exec();
}
