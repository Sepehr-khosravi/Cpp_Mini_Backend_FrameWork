#include "Request.hpp"
#include <iostream>
#include <string>
using namespace std;

#include "Deserializing.hpp"


Request Request::deserializing(std::string &raw){
    Request req = parseHttpRequest(raw, method, target, version, headers, body);

    return req; 
}