#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include "Value.hpp"

class Struct{
    private:
        std::unordered_map<std::string, Value> data;
    public :
    void set(const std::string &key, Value value);

    Value get(const std::string &key) const;

    std::unordered_map<std::string, Value>& items();
    const std::unordered_map<std::string, Value>& items() const;

    bool operator!() const;
    
};