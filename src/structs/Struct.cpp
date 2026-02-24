#include <iostream>
#include <string>
using namespace std;
#include <unordered_map>
#include "Struct.hpp"
// #include "handler.hpp"
#include "Value.hpp"

void Struct::set(const std::string &key,Value value ){
    data[key] = std::move(value);
};

Value Struct::get(const std::string &key) const {
    auto it = this->data.find(key);
    if(it == this->data.end()){
        return false;
    };
    return it->second;
};


std::unordered_map<std::string, Value>& Struct::items(){
    return data;
};

const std::unordered_map<std::string, Value>& Struct::items() const{
    return data;
};


bool Struct::operator!() const{
    return data.empty();
};