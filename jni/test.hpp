#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

class CTest1 {
public:
    CTest1() = default;
    CTest1& withString(const std::string&);
    std::string str;
};

class CTest2 {
public:
    static std::unique_ptr<CTest2> createCTest2(std::optional<CTest1>&&);
    bool isValid() const;
protected:
    CTest2(std::optional<CTest1>&&);
private:
    std::optional<CTest1> var;
};

class CTest3 {
public:
    using CFun = std::function<void()>;
    virtual void fun(CFun&&) = 0;
protected:
    CTest3() = default;
};

class CTest4 : public CTest3 {
public:
    CTest4() = default;
    void setCTest2(std::unique_ptr<CTest2>&&);
    // throws std::runtime_error if no valid CTest2 is set.
    void fun(CFun&&) final;
private:
    std::unique_ptr<CTest2> test2;
};
