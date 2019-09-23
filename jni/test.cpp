#include "test.hpp"

CTest1& CTest1::withString(const std::string& str_) {
    str = str_;
    return *this;
}

std::unique_ptr<CTest2> CTest2::createCTest2(std::optional<CTest1>&& var_) {
    return std::make_unique<CTest2>(CTest2(std::move(var_)));
}

CTest2::CTest2(std::optional<CTest1>&& var_)
    : var(std::move(var_)) {}

bool CTest2::isValid() const {
    return var && !var->str.empty();
}

void CTest4::setCTest2(std::unique_ptr<CTest2>&& test2_) {
    test2 = std::move(test2_);
}

void CTest4::fun(CFun&& callback) {
    if (test2 && test2->isValid()) {
        callback();
    } else {
        throw std::runtime_error("no valid CTest2 is set");
    }
}
