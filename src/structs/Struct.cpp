#include "Struct.hpp"
#include "Value.hpp"  

void Struct::set(const std::string& key, const Value& value) {
    data[key] = value;
}

Value Struct::get(const std::string& key) const {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return std::optional<Struct>(std::nullopt);
}

std::unordered_map<std::string, Value>& Struct::items() {
    return data;
}

const std::unordered_map<std::string, Value>& Struct::items() const {
    return data;
}

bool Struct::operator!() const {
    return data.empty();
}
