#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <variant>
#include "Struct.hpp"
#include "Value.hpp"

class Struct;

std::string serializeValue(const Value& value);
std::string serializeStruct(const Struct& data);


inline std::string serializeStruct(const Struct&);

inline std::string serializeValue(const Value& value) {
    return std::visit([](auto&& v) -> std::string {
        using T = std::decay_t<decltype(v)>;

        if constexpr (std::is_same_v<T, int>) {
            return std::to_string(v);
        }
        else if constexpr (std::is_same_v<T, double>) {
            return std::to_string(v);
        }
        else if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return "\"" + v + "\"";
        }
        else if constexpr (
            std::is_same_v<T,
            std::unordered_map<std::string, std::shared_ptr<Struct>>>
        ) {
            std::string out = "{";
            bool first = true;
            for (const auto& [k, ptr] : v) {
                if (!first) out += ",";
                first = false;
                out += "\"" + k + "\":";
                out += ptr ? serializeStruct(*ptr) : "null";
            }
            out += "}";
            return out;
        }
        else if constexpr (std::is_same_v<T, Struct>) {
            return serializeStruct(v);
        }
        else {
            return "null";
        }
    }, value);
}

inline std::string serializeStruct(const Struct& data) {
    std::string out = "{";
    bool first = true;

    for (const auto& [key, value] : data.items()) {
        if (!first) out += ",";
        first = false;
        out += "\"" + key + "\":" + serializeValue(value);
    }

    out += "}";
    return out;
}