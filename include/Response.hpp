#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <memory>
#include "Value.hpp"
#include "Struct.hpp"
#include "Serializing.hpp"
#include "ResponseData.hpp"



class Response{
    public :
        std::string setStatusText() const;
    public:

        int status;
        std::string status_text = setStatusText();
        Struct headers;
        std::string body;
    public:
        std::string parse_response(Value data);
        std::string json(Value data);

        void setHeader(std::string &key, std::string &value);

    
        std::string serializing(ResponseData &data);



    
};