#pragma once
#include <variant>
#include <string>
#include <unordered_map>
#include <memory>
// #include "Struct.hpp"

class Struct;


using Value = std::variant<
    int,
    double,
    std::string,
    bool,
    std::unordered_map<std::string, Struct>,
    Struct
>;


std::string serializeValue(const Value& value);
std::string serializeStruct(const Struct& data);