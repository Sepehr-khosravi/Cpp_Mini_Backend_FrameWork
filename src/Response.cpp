#include "Response.hpp"
#include <iostream>
#include <string>
using namespace std;
#include <memory>
#include "Value.hpp"
#include "Struct.hpp"
#include <unordered_map>
#include "Serializing.hpp"
#include "ResponseData.hpp"


void Response::setHeader(std::string &key, std::string &value ){
    headers.set(key, value);
};


std::string Response::json(Value data){
    headers.set("Contetnt-Type", "applicaton/json");

    body = parse_response(data);

    ResponseData result;

    result.status = this->status;
    result.body = this->body;
    result.headers = this->headers;
    result.status_text = this->status_text;
    result.version = "HTTP/1.1";

    return serializing(result);

};


std::string Response::parse_response(Value data){
    return serializeValue(data);
};

std::string Response::serializing(ResponseData &data){
    std::string headerString;
    for(const auto& [key, value] : this->headers.items()){
        headerString += key + " : " + "\"" + serializeValue(value) + "\"" + "\r\n";
    };
    std::string Content;
    Content = data.version + " " + std::to_string(data.status) + " " +  data.status_text + "\r\n" + headerString + "\r\n" + data.body ; 
    return Content;
}


std::string Response::setStatusText() const {
    switch(status) {
                case 200 : return "OK";
                case 500 : return "Internal Server Error";
                case 404 : return "Not Found Error";
                case 401 : return "Unauthorization";
    };
    return "unknown";
};