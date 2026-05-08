#pragma once
#include <string>
#include "Struct.hpp"

struct ResponseData {
    std::string version;
    int status;
    std::string status_text;
    Struct headers;
    std::string body;
};
