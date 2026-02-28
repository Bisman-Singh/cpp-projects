#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <limits>

struct Transaction {
    std::string timestamp;
    std::string type;
    double amount;
    double balance_after;
    std::string description;
};

class BankAccount {
public:
    BankAccount() = default;
    BankAccount(int id, std::string name, double initial_balance)
        : id_(id), name_(std::move(name)), balance_(initial_balance) {
        if (initial_balance > 0) {
            record_transaction("DEPOSIT", initial_balance, "Initial deposit");
        }
    }

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    double balance() const { return balance_; }
    const std::vector<Transaction>& history() const { return history_; }

    bool deposit(double amount) {
        if (amount <= 0) return false;
        balance_ += amount;
        record_transaction("DEPOSIT", amount, "Cash deposit");
        return true;
    }

    bool withdraw(double amount) {
        if (amount <= 0 || amount > balance_) return false;
        balance_ -= amount;
        record_transaction("WITHDRAW", amount, "Cash withdrawal");
        return true;
    }

    bool transfer_out(double amount, int dest_id) {
        if (amount <= 0 || amount > balance_) return false;
        balance_ -= amount;
        record_transaction("TRANSFER_OUT", amount, "Transfer to account #" + std::to_string(dest_id));
        return true;
    }

    void transfer_in(double amount, int src_id) {
        balance_ += amount;
        record_transaction("TRANSFER_IN", amount, "Transfer from account #" + std::to_string(src_id));
    }

    void serialize(std::ofstream& out) const {
        out << id_ << '\n' << name_ << '\n' << std::fixed << std::setprecision(2) << balance_ << '\n';
        out << history_.size() << '\n';
        for (const auto& t : history_) {
            out << t.timestamp << '|' << t.type << '|'
                << std::fixed << std::setprecision(2) << t.amount << '|'
                << t.balance_after << '|' << t.description << '\n';
        }
    }

    static BankAccount deserialize(std::ifstream& in) {
        BankAccount acc;
        std::string line;

        std::getline(in, line);
        acc.id_ = std::stoi(line);
        std::getline(in, acc.name_);
        std::getline(in, line);
        acc.balance_ = std::stod(line);

        std::getline(in, line);
        int count = std::stoi(line);
        for (int i = 0; i < count; ++i) {
            std::getline(in, line);
            Transaction t;
            std::istringstream ss(line);
            std::string token;
            std::getline(ss, t.timestamp, '|');
            std::getline(ss, t.type, '|');
            std::getline(ss, token, '|');
            t.amount = std::stod(token);
            std::getline(ss, token, '|');
            t.balance_after = std::stod(token);
            std::getline(ss, t.description, '|');
            acc.history_.push_back(std::move(t));
        }
        return acc;
    }

private:
    int id_ = 0;
    std::string name_;
    double balance_ = 0.0;
    std::vector<Transaction> history_;

    void record_transaction(const std::string& type, double amount, const std::string& desc) {
        auto now = std::time(nullptr);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        history_.push_back({buf, type, amount, balance_, desc});
    }
};

class Bank {
public:
    explicit Bank(const std::string& filename) : filename_(filename) {
        load();
    }

    ~Bank() { save(); }

    int create_account(const std::string& name, double initial_balance) {
        int id = next_id_++;
        accounts_.emplace(id, BankAccount(id, name, initial_balance));
        save();
        return id;
    }

    BankAccount* find_account(int id) {
        auto it = accounts_.find(id);
        return it != accounts_.end() ? &it->second : nullptr;
    }

    bool deposit(int id, double amount) {
        auto* acc = find_account(id);
        if (!acc) return false;
        bool ok = acc->deposit(amount);
        if (ok) save();
        return ok;
    }

    bool withdraw(int id, double amount) {
        auto* acc = find_account(id);
        if (!acc) return false;
        bool ok = acc->withdraw(amount);
        if (ok) save();
        return ok;
    }

    bool transfer(int from_id, int to_id, double amount) {
        auto* from = find_account(from_id);
        auto* to = find_account(to_id);
        if (!from || !to || from_id == to_id) return false;
        if (!from->transfer_out(amount, to_id)) return false;
        to->transfer_in(amount, from_id);
        save();
        return true;
    }

    void list_accounts() const {
        if (accounts_.empty()) {
            std::cout << "No accounts found.\n";
            return;
        }
        std::cout << std::left << std::setw(8) << "ID"
                  << std::setw(25) << "Name"
                  << std::right << std::setw(15) << "Balance" << '\n';
        std::cout << std::string(48, '-') << '\n';
        for (const auto& [id, acc] : accounts_) {
            std::cout << std::left << std::setw(8) << id
                      << std::setw(25) << acc.name()
                      << std::right << std::setw(12) << "$"
                      << std::fixed << std::setprecision(2) << acc.balance() << '\n';
        }
    }

    void show_history(int id) const {
        auto it = accounts_.find(id);
        if (it == accounts_.end()) {
            std::cout << "Account not found.\n";
            return;
        }
        const auto& hist = it->second.history();
        if (hist.empty()) {
            std::cout << "No transactions.\n";
            return;
        }
        std::cout << std::left
                  << std::setw(22) << "Timestamp"
                  << std::setw(15) << "Type"
                  << std::setw(12) << "Amount"
                  << std::setw(12) << "Balance"
                  << "Description\n";
        std::cout << std::string(75, '-') << '\n';
        for (const auto& t : hist) {
            std::cout << std::left
                      << std::setw(22) << t.timestamp
                      << std::setw(15) << t.type
                      << "$" << std::right << std::setw(10) << std::fixed << std::setprecision(2) << t.amount
                      << "  $" << std::setw(10) << t.balance_after
                      << "  " << t.description << '\n';
        }
    }

private:
    std::string filename_;
    std::map<int, BankAccount> accounts_;
    int next_id_ = 1;

    void save() const {
        std::ofstream out(filename_);
        if (!out) {
            std::cerr << "Error: could not save to " << filename_ << '\n';
            return;
        }
        out << next_id_ << '\n' << accounts_.size() << '\n';
        for (const auto& [id, acc] : accounts_) {
            acc.serialize(out);
        }
    }

    void load() {
        std::ifstream in(filename_);
        if (!in) return;

        std::string line;
        std::getline(in, line);
        if (line.empty()) return;
        next_id_ = std::stoi(line);

        std::getline(in, line);
        int count = std::stoi(line);
        for (int i = 0; i < count; ++i) {
            auto acc = BankAccount::deserialize(in);
            accounts_.emplace(acc.id(), std::move(acc));
        }
    }
};

static void clear_input() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static int read_int(const std::string& prompt) {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) {
            clear_input();
            return val;
        }
        clear_input();
        std::cout << "Invalid input. Please enter a number.\n";
    }
}

static double read_double(const std::string& prompt) {
    double val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val && val >= 0) {
            clear_input();
            return val;
        }
        clear_input();
        std::cout << "Invalid input. Please enter a valid amount.\n";
    }
}

int main() {
    Bank bank("accounts.dat");

    while (true) {
        std::cout << "\n===== Bank Management System =====\n"
                  << "1. Create Account\n"
                  << "2. Deposit\n"
                  << "3. Withdraw\n"
                  << "4. Transfer\n"
                  << "5. View Balance\n"
                  << "6. Transaction History\n"
                  << "7. List All Accounts\n"
                  << "8. Exit\n";

        int choice = read_int("Choose option: ");

        switch (choice) {
        case 1: {
            std::cout << "Enter account holder name: ";
            std::string name;
            std::getline(std::cin, name);
            double balance = read_double("Initial deposit amount: $");
            int id = bank.create_account(name, balance);
            std::cout << "Account created. ID: " << id << '\n';
            break;
        }
        case 2: {
            int id = read_int("Account ID: ");
            double amt = read_double("Deposit amount: $");
            if (bank.deposit(id, amt))
                std::cout << "Deposit successful.\n";
            else
                std::cout << "Deposit failed. Check account ID and amount.\n";
            break;
        }
        case 3: {
            int id = read_int("Account ID: ");
            double amt = read_double("Withdraw amount: $");
            if (bank.withdraw(id, amt))
                std::cout << "Withdrawal successful.\n";
            else
                std::cout << "Withdrawal failed. Check account ID and balance.\n";
            break;
        }
        case 4: {
            int from = read_int("From Account ID: ");
            int to = read_int("To Account ID: ");
            double amt = read_double("Transfer amount: $");
            if (bank.transfer(from, to, amt))
                std::cout << "Transfer successful.\n";
            else
                std::cout << "Transfer failed. Check account IDs and balance.\n";
            break;
        }
        case 5: {
            int id = read_int("Account ID: ");
            auto* acc = bank.find_account(id);
            if (acc)
                std::cout << "Account: " << acc->name() << " | Balance: $"
                          << std::fixed << std::setprecision(2) << acc->balance() << '\n';
            else
                std::cout << "Account not found.\n";
            break;
        }
        case 6: {
            int id = read_int("Account ID: ");
            bank.show_history(id);
            break;
        }
        case 7:
            bank.list_accounts();
            break;
        case 8:
            std::cout << "Goodbye!\n";
            return 0;
        default:
            std::cout << "Invalid option.\n";
        }
    }
}
