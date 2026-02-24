#pragma once
#include <iostream>
#include <string>
#include "Struct.hpp"
#include "Value.hpp"
#include <unordered_map>





class Request{
    public :
        std::string method;
        std::string target;
        std::string version;
        Struct headers;
        Struct body;
    

    Request deserializing(std::string &raw);
};
