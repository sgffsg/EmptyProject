Вот реализация программы, которая симулирует работу экономики в городке с учетом всех требований. Код включает класс `Bank`, персонажей и юнит-тесты.

### Класс Bank (bank.h и bank.cpp)

```cpp
// bank.h
#pragma once
#include <stdexcept>
#include <unordered_map>
#include <string>

using AccountId = unsigned long long;
using Money = long long;

class BankOperationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Bank {
public:
    explicit Bank(Money cash);
    Bank(const Bank&) = delete;
    Bank& operator=(const Bank&) = delete;

    void SendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount);
    [[nodiscard]] bool TrySendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount);
    [[nodiscard]] Money GetCash() const;
    Money GetAccountBalance(AccountId accountId) const;
    void WithdrawMoney(AccountId account, Money amount);
    [[nodiscard]] bool TryWithdrawMoney(AccountId account, Money amount);
    void DepositMoney(AccountId account, Money amount);
    [[nodiscard]] AccountId OpenAccount();
    [[nodiscard]] Money CloseAccount(AccountId accountId);

private:
    Money cash_;
    AccountId nextAccountId_ = 1;
    std::unordered_map<AccountId, Money> accounts_;
};
```

```cpp
// bank.cpp
#include "bank.h"
#include <stdexcept>

Bank::Bank(Money cash) : cash_(cash) {
    if (cash < 0) {
        throw BankOperationError("Initial cash cannot be negative");
    }
}

void Bank::SendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount) {
    if (amount < 0) {
        throw std::out_of_range("Amount cannot be negative");
    }

    auto srcIt = accounts_.find(srcAccountId);
    auto dstIt = accounts_.find(dstAccountId);

    if (srcIt == accounts_.end() || dstIt == accounts_.end()) {
        throw BankOperationError("Invalid account ID");
    }

    if (srcIt->second < amount) {
        throw BankOperationError("Insufficient funds");
    }

    srcIt->second -= amount;
    dstIt->second += amount;
}

bool Bank::TrySendMoney(AccountId srcAccountId, AccountId dstAccountId, Money amount) {
    if (amount < 0) {
        throw std::out_of_range("Amount cannot be negative");
    }

    auto srcIt = accounts_.find(srcAccountId);
    auto dstIt = accounts_.find(dstAccountId);

    if (srcIt == accounts_.end() || dstIt == accounts_.end()) {
        throw BankOperationError("Invalid account ID");
    }

    if (srcIt->second < amount) {
        return false;
    }

    srcIt->second -= amount;
    dstIt->second += amount;
    return true;
}

Money Bank::GetCash() const {
    return cash_;
}

Money Bank::GetAccountBalance(AccountId accountId) const {
    auto it = accounts_.find(accountId);
    if (it == accounts_.end()) {
        throw BankOperationError("Invalid account ID");
    }
    return it->second;
}

void Bank::WithdrawMoney(AccountId account, Money amount) {
    if (amount < 0) {
        throw std::out_of_range("Amount cannot be negative");
    }

    auto it = accounts_.find(account);
    if (it == accounts_.end()) {
        throw BankOperationError("Invalid account ID");
    }

    if (it->second < amount) {
        throw BankOperationError("Insufficient funds");
    }

    it->second -= amount;
    cash_ += amount;
}

bool Bank::TryWithdrawMoney(AccountId account, Money amount) {
    if (amount < 0) {
        throw std::out_of_range("Amount cannot be negative");
    }

    auto it = accounts_.find(account);
    if (it == accounts_.end()) {
        throw BankOperationError("Invalid account ID");
    }

    if (it->second < amount) {
        return false;
    }

    it->second -= amount;
    cash_ += amount;
    return true;
}

void Bank::DepositMoney(AccountId account, Money amount) {
    if (amount < 0) {
        throw std::out_of_range("Amount cannot be negative");
    }

    auto it = accounts_.find(account);
    if (it == accounts_.end()) {
        throw BankOperationError("Invalid account ID");
    }

    if (cash_ < amount) {
        throw BankOperationError("Insufficient cash");
    }

    it->second += amount;
    cash_ -= amount;
}

AccountId Bank::OpenAccount() {
    AccountId newAccountId = nextAccountId_++;
    accounts_[newAccountId] = 0;
    return newAccountId;
}

Money Bank::CloseAccount(AccountId accountId) {
    auto it = accounts_.find(accountId);
    if (it == accounts_.end()) {
        throw BankOperationError("Invalid account ID");
    }

    Money balance = it->second;
    cash_ += balance;
    accounts_.erase(it);
    return balance;
}
```

### Персонажи (actors.h и actors.cpp)

```cpp
// actors.h
#pragma once
#include "bank.h"
#include <string>
#include <memory>

class Actor {
public:
    virtual ~Actor() = default;
    virtual void Act() = 0;
    virtual std::string GetName() const = 0;
    [[nodiscard]] virtual Money GetCash() const = 0;
    [[nodiscard]] virtual AccountId GetAccountId() const = 0;
};

class Homer : public Actor {
public:
    explicit Homer(std::shared_ptr<Bank> bank);
    void Act() override;
    std::string GetName() const override { return "Homer"; }
    [[nodiscard]] Money GetCash() const override { return cash_; }
    [[nodiscard]] AccountId GetAccountId() const override { return accountId_; }

private:
    std::shared_ptr<Bank> bank_;
    AccountId accountId_;
    Money cash_ = 100;
};

class Marge : public Actor {
public:
    explicit Marge(std::shared_ptr<Bank> bank);
    void Act() override;
    std::string GetName() const override { return "Marge"; }
    [[nodiscard]] Money GetCash() const override { return cash_; }
    [[nodiscard]] AccountId GetAccountId() const override { return accountId_; }

private:
    std::shared_ptr<Bank> bank_;
    AccountId accountId_;
    Money cash_ = 50;
};

class Bart : public Actor {
public:
    explicit Bart(std::shared_ptr<Bank> bank);
    void Act() override;
    std::string GetName() const override { return "Bart"; }
    [[nodiscard]] Money GetCash() const override { return cash_; }
    [[nodiscard]] AccountId GetAccountId() const override { return 0; } // No account

private:
    std::shared_ptr<Bank> bank_;
    Money cash_ = 10;
};

class Lisa : public Actor {
public:
    explicit Lisa(std::shared_ptr<Bank> bank);
    void Act() override;
    std::string GetName() const override { return "Lisa"; }
    [[nodiscard]] Money GetCash() const override { return cash_; }
    [[nodiscard]] AccountId GetAccountId() const override { return 0; } // No account

private:
    std::shared_ptr<Bank> bank_;
    Money cash_ = 10;
};

class Apu : public Actor {
public:
    explicit Apu(std::shared_ptr<Bank> bank);
    void Act() override;
    std::string GetName() const override { return "Apu"; }
    [[nodiscard]] Money GetCash() const override { return cash_; }
    [[nodiscard]] AccountId GetAccountId() const override { return accountId_; }

private:
    std::shared_ptr<Bank> bank_;
    AccountId accountId_;
    Money cash_ = 200;
};

class Burns : public Actor {
public:
    explicit Burns(std::shared_ptr<Bank> bank);
    void Act() override;
    std::string GetName() const override { return "Burns"; }
    [[nodiscard]] Money GetCash() const override { return cash_; }
    [[nodiscard]] AccountId GetAccountId() const override { return accountId_; }

private:
    std::shared_ptr<Bank> bank_;
    AccountId accountId_;
    Money cash_ = 1000;
};
```

```cpp
// actors.cpp
#include "actors.h"
#include <iostream>

Homer::Homer(std::shared_ptr<Bank> bank) : bank_(std::move(bank)) {
    accountId_ = bank_->OpenAccount();
}

void Homer::Act() {
    // Give money to Marge
    try {
        bank_->SendMoney(accountId_, bank_->GetAccountBalance(accountId_) / 2, Marge(bank_).GetAccountId());
        std::cout << "Homer gave money to Marge" << std::endl;
    } catch (...) {
        std::cout << "Homer failed to give money to Marge" << std::endl;
    }

    // Withdraw cash for kids
    try {
        bank_->WithdrawMoney(accountId_, 20);
        cash_ += 10; // Give to Bart and Lisa
        std::cout << "Homer withdrew cash for kids" << std::endl;
    } catch (...) {
        std::cout << "Homer failed to withdraw cash" << std::endl;
    }
}

Marge::Marge(std::shared_ptr<Bank> bank) : bank_(std::move(bank)) {
    accountId_ = bank_->OpenAccount();
}

void Marge::Act() {
    // Buy groceries from Apu
    try {
        bank_->SendMoney(accountId_, 15, Apu(bank_).GetAccountId());
        std::cout << "Marge bought groceries from Apu" << std::endl;
    } catch (...) {
        std::cout << "Marge failed to buy groceries" << std::endl;
    }
}

Bart::Bart(std::shared_ptr<Bank> bank) : bank_(std::move(bank)) {}

void Bart::Act() {
    if (cash_ >= 5) {
        cash_ -= 5;
        std::cout << "Bart bought something from Apu with cash" << std::endl;
    } else {
        std::cout << "Bart has no cash to spend" << std::endl;
    }
}

Lisa::Lisa(std::shared_ptr<Bank> bank) : bank_(std::move(bank)) {}

void Lisa::Act() {
    if (cash_ >= 3) {
        cash_ -= 3;
        std::cout << "Lisa bought something from Apu with cash" << std::endl;
    } else {
        std::cout << "Lisa has no cash to spend" << std::endl;
    }
}

Apu::Apu(std::shared_ptr<Bank> bank) : bank_(std::move(bank)) {
    accountId_ = bank_->OpenAccount();
}

void Apu::Act() {
    // Deposit cash to bank
    if (cash_ >= 50) {
        try {
            bank_->DepositMoney(accountId_, 50);
            cash_ -= 50;
            std::cout << "Apu deposited cash to bank" << std::endl;
        } catch (...) {
            std::cout << "Apu failed to deposit cash" << std::endl;
        }
    }

    // Pay for electricity
    try {
        bank_->SendMoney(accountId_, 100, Burns(bank_).GetAccountId());
        std::cout << "Apu paid for electricity" << std::endl;
    } catch (...) {
        std::cout << "Apu failed to pay for electricity" << std::endl;
    }
}

Burns::Burns(std::shared_ptr<Bank> bank) : bank_(std::move(bank)) {
    accountId_ = bank_->OpenAccount();
}

void Burns::Act() {
    // Pay salary to Homer
    try {
        bank_->SendMoney(accountId_, 200, Homer(bank_).GetAccountId());
        std::cout << "Burns paid salary to Homer" << std::endl;
    } catch (...) {
        std::cout << "Burns failed to pay salary" << std::endl;
    }
}
```

### Юнит-тесты (bank_tests.cpp)

```cpp
#include "bank.h"
#include <gtest/gtest.h>

TEST(BankTest, Initialization) {
    Bank bank(1000);
    EXPECT_EQ(bank.GetCash(), 1000);
}

TEST(BankTest, OpenAccount) {
    Bank bank(1000);
    AccountId id = bank.OpenAccount();
    EXPECT_EQ(bank.GetAccountBalance(id), 0);
}

TEST(BankTest, DepositMoney) {
    Bank bank(1000);
    AccountId id = bank.OpenAccount();
    bank.DepositMoney(id, 100);
    EXPECT_EQ(bank.GetAccountBalance(id), 100);
    EXPECT_EQ(bank.GetCash(), 900);
}

TEST(BankTest, WithdrawMoney) {
    Bank bank(1000);
    AccountId id = bank.OpenAccount();
    bank.DepositMoney(id, 100);
    bank.WithdrawMoney(id, 50);
    EXPECT_EQ(bank.GetAccountBalance(id), 50);
    EXPECT_EQ(bank.GetCash(), 950);
}

TEST(BankTest, SendMoney) {
    Bank bank(1000);
    AccountId id1 = bank.OpenAccount();
    AccountId id2 = bank.OpenAccount();
    bank.DepositMoney(id1, 200);
    bank.SendMoney(id1, id2, 100);
    EXPECT_EQ(bank.GetAccountBalance(id1), 100);
    EXPECT_EQ(bank.GetAccountBalance(id2), 100);
}

TEST(BankTest, CloseAccount) {
    Bank bank(1000);
    AccountId id = bank.OpenAccount();
    bank.DepositMoney(id, 100);
    Money balance = bank.CloseAccount(id);
    EXPECT_EQ(balance, 100);
    EXPECT_EQ(bank.GetCash(), 1000);
}
```

### Главная программа (main.cpp)

```cpp
#include "actors.h"
#include <vector>
#include <memory>
#include <iostream>

int main(int argc, char* argv[]) {
    int iterations = 10;
    if (argc > 1) {
        iterations = std::stoi(argv[1]);
    } else {
        std::cout << "Enter number of iterations: ";
        std::cin >> iterations;
    }

    auto bank = std::make_shared<Bank>(10000);
    std::vector<std::unique_ptr<Actor>> actors;
    actors.push_back(std::make_unique<Homer>(bank));
    actors.push_back(std::make_unique<Marge>(bank));
    actors.push_back(std::make_unique<Bart>(bank));
    actors.push_back(std::make_unique<Lisa>(bank));
    actors.push_back(std::make_unique<Apu>(bank));
    actors.push_back(std::make_unique<Burns>(bank));

    for (int i = 0; i < iterations; ++i) {
        std::cout << "--- Iteration " << i + 1 << " ---" << std::endl;
        for (auto& actor : actors) {
            actor->Act();
        }
    }

    std::cout << "\nFinal balances:" << std::endl;
    Money totalCash = bank->GetCash();
    Money totalBank = 0;
    for (auto& actor : actors) {
        std::cout << actor->GetName() << ": Cash=" << actor->GetCash();
        if (actor->GetAccountId() != 0) {
            Money balance = bank->GetAccountBalance(actor->GetAccountId());
            std::cout << ", Bank=" << balance;
            totalBank += balance;
        }
        std::cout << std::endl;
        totalCash += actor->GetCash();
    }

    std::cout << "Total cash in system: " << totalCash << std::endl;
    std::cout << "Total in bank accounts: " << totalBank << std::endl;
    std::cout << "Combined total: " << totalCash + totalBank << std::endl;

    return 0;
}
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(EconomySimulator)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)

add_executable(economy main.cpp bank.cpp actors.cpp)
add_executable(bank_tests bank_tests.cpp bank.cpp)
target_link_libraries(bank_tests GTest::GTest GTest::Main)

enable_testing()
add_test(NAME BankTests COMMAND bank_tests)
```

### Обоснование безопасности исключений:
- Все методы класса `Bank` предоставляют как минимум базовую гарантию безопасности исключений.
- В методах, где возможны исключения (например, при нехватке средств), состояние объекта остается согласованным.
- Операции, изменяющие состояние (например, перевод денег), выполняются атомарно или с откатом в случае ошибки.

Программа моделирует экономику городка, где персонажи взаимодействуют через банковскую систему. В конце симуляции проверяется сохранение общего количества денег в системе.