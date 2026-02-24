#pragma once

#include <string>
#include <cctype>
#include <cstdlib>
#include "Struct.hpp"

class Request;

inline std::string deserializingMethod(const std::string& raw, size_t& methodPos) {
    methodPos = raw.find(' ');
    if (methodPos == std::string::npos)
        return "";
    return raw.substr(0, methodPos);
}

inline std::string deserializingTarget(
    const std::string& raw,
    size_t methodPos,
    size_t& targetPos
) {
    if (methodPos == std::string::npos)
        return "";

    targetPos = raw.find(' ', methodPos + 1);
    if (targetPos == std::string::npos)
        return "";

    return raw.substr(methodPos + 1, targetPos - methodPos - 1);
}

inline std::string deserializingVersion(
    const std::string& raw,
    size_t targetPos,
    size_t& versionPos
) {
    if (targetPos == std::string::npos)
        return "";

    versionPos = raw.find("\r\n");
    if (versionPos == std::string::npos)
        return "";

    return raw.substr(targetPos + 1, versionPos - targetPos - 1);
}



inline Struct deserializingHeaders(const std::string& raw) {
    Struct headers;

    size_t headersEnd = raw.find("\r\n\r\n");
    if (headersEnd == std::string::npos)
        return headers;

    size_t requestLineEnd = raw.find("\r\n");
    if (requestLineEnd == std::string::npos)
        return headers;

    std::string section =
        raw.substr(requestLineEnd + 2, headersEnd - requestLineEnd - 2);

    size_t pos = 0;
    while (pos < section.size()) {
        size_t end = section.find("\r\n", pos);
        if (end == std::string::npos)
            end = section.size();

        std::string line = section.substr(pos, end - pos);
        size_t colon = line.find(':');

        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);

            size_t first = value.find_first_not_of(" \t");
            size_t last  = value.find_last_not_of(" \t\r\n");
            if (first != std::string::npos && last != std::string::npos)
                value = value.substr(first, last - first + 1);

            headers.set(key, value);
        }

        pos = end + 2;
    }

    return headers;
}



inline size_t findMatchingClosingBrace(const std::string& s, size_t start) {
    int depth = 1;
    for (size_t i = start + 1; i < s.size(); i++) {
        if (s[i] == '{') depth++;
        else if (s[i] == '}') {
            depth--;
            if (depth == 0) return i;
        }
    }
    return std::string::npos;
}

inline void skipWs(const std::string& s, size_t& pos) {
    while (pos < s.size() &&
           std::isspace(static_cast<unsigned char>(s[pos])))
        pos++;
}

inline std::string parseString(const std::string& s, size_t& pos) {
    std::string out;
    pos++;

    while (pos < s.size()) {
        char c = s[pos++];
        if (c == '\\') {
            if (pos < s.size())
                out += s[pos++];
        }
        else if (c == '"') {
            break;
        }
        else {
            out += c;
        }
    }
    return out;
}

inline Value parsePrimitive(const std::string& s, size_t& pos) {
    size_t start = pos;
    while (pos < s.size() && s[pos] != ',' && s[pos] != '}')
        pos++;

    std::string tok = s.substr(start, pos - start);

    if (tok == "true")  return true;
    if (tok == "false") return false;

    char* end = nullptr;
    long i = std::strtol(tok.c_str(), &end, 10);
    if (*end == '\0')
        return static_cast<int>(i);

    double d = std::strtod(tok.c_str(), &end);
    if (*end == '\0')
        return d;

    return tok;
}



inline Struct parseJsonObject(const std::string& json, size_t& pos) {
    Struct obj;
    pos++;

    while (pos < json.size()) {
        skipWs(json, pos);

        if (json[pos] == '}') {
            pos++;
            break;
        }

        std::string key = parseString(json, pos);

        skipWs(json, pos);
        pos++; // skip ':'

        skipWs(json, pos);
        if (json[pos] == '"') {
            obj.set(key, parseString(json, pos));
        }
        else if (json[pos] == '{') {
            obj.set(key, parseJsonObject(json, pos));
        }
        else {
            obj.set(key, parsePrimitive(json, pos));
        }

        skipWs(json, pos);
        if (json[pos] == ',')
            pos++;
    }

    return obj;
}



inline Struct deserializingBody(const std::string& raw) {
    size_t start = raw.find('{');
    if (start == std::string::npos)
        return {};

    size_t end = findMatchingClosingBrace(raw, start);
    if (end == std::string::npos)
        return {};

    std::string json = raw.substr(start, end - start + 1);
    size_t pos = 0;
    return parseJsonObject(json, pos);
};



inline Request parseHttpRequest(
    const std::string& rawRequest,
    std::string& method,
    std::string& target,
    std::string& version,
    Struct& headers,
    Struct& body
) {
    size_t methodPos, targetPos, versionPos;

    method  = deserializingMethod(rawRequest, methodPos);
    target  = deserializingTarget(rawRequest, methodPos, targetPos);
    version = deserializingVersion(rawRequest, targetPos, versionPos);

    headers = deserializingHeaders(rawRequest);
    body    = deserializingBody(rawRequest);
    
    Request req;
    req.method = method;
    req.target = target;
    req.body = body;
    req.headers = headers;
    req.version = version;
    
    return req;
    
};
