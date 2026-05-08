#pragma once
#include <string>
#include "Value.hpp"
#include "Struct.hpp"
#include "ResponseData.hpp"

class Response {
public:
    int status = 200;
    std::string status_text;
    Struct headers;
    std::string body;
    std::string htmlContent;
    bool pageError = false;
    bool hasPage = false;
    
    std::string setStatusText() const;
    std::string parse_response(const Value& data);  
    std::string json(const Value& data);            
    std::string send(const Value& data, const std::string& page);  
    void setHeader(const std::string& key, const std::string& value);
    void render(const std::string& htmlAddress);
    std::string load();
    bool checkPageError();
    std::string serializing(ResponseData& data);
};
