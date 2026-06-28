#pragma once
#include <string>
#include "Value.hpp"
#include "Struct.hpp"

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
            return v;
        }
        else if constexpr (std::is_same_v<T, std::unordered_map<std::string, Struct>>) {
            std::string out = "{";
            bool first = true;
            for (const auto& [k, struct_val] : v) {
                if (!first) out += ",";
                first = false;
                out += "\"" + k + "\":" + serializeStruct(struct_val);
            }
            out += "}";
            return out;
        }
        else if constexpr (std::is_same_v<T, Struct>) {
            return serializeStruct(v);
        }
        else if constexpr (std::is_same_v<T, std::optional<Struct>>) {
            if (!v.has_value()) {
                std::cerr << "Serializing Data Error : you must return at least something to take back data." << std::endl;
                return "null";
            }
            return serializeStruct(v.value());
        }
        else {
            return "null";
        }
    }, value);
}

inline std::string serializeStruct(const Struct& s) {
    std::string out = "{";
    bool first = true;
    
    for (const auto& [key, value] : s.items()) {
        if (!first) out += ",";
        first = false;
        out += key + ":" + serializeValue(value); 
    }
    
    out += "}";
    return out;
}

inline std::string serializeStruct(const std::optional<Struct>& data) {
    if (data.has_value()) {
        return serializeStruct(data.value());
    }
    return "";
}