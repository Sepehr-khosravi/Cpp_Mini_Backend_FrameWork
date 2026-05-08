#pragma once
#include <variant>
#include <string>
#include <unordered_map>
#include <memory>
#include <optional>

// Forward declaration of Struct
struct Struct;

using Value = std::variant<
    int,
    double,
    std::string,
    bool,
    std::unordered_map<std::string, Struct>,
    Struct,
    std::optional<Struct>
>;

// Function declarations
std::string serializeValue(const Value& value);
std::string serializeStruct(const Struct& data);
std::string serializeStruct(const std::optional<Struct>& data);
