#include "Response.hpp"
#include "Struct.hpp"
#include "Serializing.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>

void Response::setHeader(const std::string& key, const std::string& value) {
    headers.set(key, value);
}

std::string Response::json(const Value& data) {
    headers.set("Content-Type", "application/json");
    
    body = parse_response(data);
    
    ResponseData result;
    result.status = this->status;
    result.body = this->body;
    result.headers = this->headers;
    result.status_text = this->status_text;
    result.version = "HTTP/1.1";
    
    return serializing(result);
}

std::string Response::send(const Value& data, const std::string& page) {
    headers.set("Content-Type", "text/html");
    headers.set("charset", "utf-8");
    
    ResponseData result;
    result.status = this->status;
    result.body = page;
    result.headers = this->headers;
    result.status_text = this->status_text;
    result.version = "HTTP/1.1";
    
    return serializing(result);
}

std::string Response::parse_response(const Value& data) {
    return serializeValue(data);
}

std::string Response::serializing(ResponseData& data) {
    std::string headerString;
    for (const auto& [key, value] : headers.items()) {
        headerString += key + ": " + serializeValue(value) + "\r\n";
    }
    
    std::string Content;
    Content = data.version + " " + std::to_string(data.status) + " " + data.status_text + "\r\n" 
              + headerString + "\r\n" + data.body;
    return Content;
}

std::string Response::setStatusText() const {
    switch(status) {
        case 200: return "OK";
        case 500: return "Internal Server Error";
        case 404: return "Not Found Error";
        case 401: return "Unauthorized";
        default: return "Unknown";
    }
}

#include "Path.hpp"

void Response::render(const std::string& htmlAddress) {
    try {
        std::string fileFormat = ".html";
        if (htmlAddress.find(fileFormat) == std::string::npos) {
            throw std::runtime_error("you must give a html file for rendering page in " + htmlAddress + " file! please try again");
        }

        std::string fullAddress = getPath() + htmlAddress;
        
        std::ifstream htmlFile(fullAddress);
        if (htmlFile.is_open()) {
            std::string content;
            std::string line;
            while (std::getline(htmlFile, line)) {
                content += line + "\n";
            }
            this->htmlContent = content;
            htmlFile.close();
            this->hasPage = true;
        } else {
            throw std::runtime_error("Cannot open file: " + fullAddress);
        }
    }
    catch(const std::runtime_error& e) {
        std::cerr << "Render Page Error : " << e.what() << std::endl;
        this->pageError = true;
    }
}

std::string Response::load() {
    return this->htmlContent;
}

bool Response::checkPageError() {
    return this->pageError;
}
