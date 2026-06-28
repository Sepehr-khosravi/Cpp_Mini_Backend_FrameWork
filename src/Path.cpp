#include "Path.hpp"
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
#include <string>


std::string getPath(){
    fs::path main_dir = fs::current_path();

    size_t pathLen = std::string(main_dir).length();
    size_t morePathLen = std::string("/external/core/health").length();

    std::string result = std::string(main_dir).substr(0, pathLen - morePathLen );


    return result;
};

