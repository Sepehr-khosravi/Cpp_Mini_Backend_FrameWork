#pragma once

struct ResponseData{
    int status;
    std::string status_text;
    Struct headers;
    std::string body;
    std::string version;
};