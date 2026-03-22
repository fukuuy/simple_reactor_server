#include "HttpParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool HttpParser::parseRequest(const string& raw_data, HttpRequest& req) 
{
    istringstream iss(raw_data);
    string line;

    if (!getline(iss, line)) return false;
    size_t pos1 = line.find(' ');
    size_t pos2 = line.find(' ', pos1 + 1);
    if (pos1 == string::npos || pos2 == string::npos) return false;

    req.method = line.substr(0, pos1);
    req.path = line.substr(pos1 + 1, pos2 - pos1 - 1);
    req.version = line.substr(pos2 + 1);

    while (getline(iss, line) && line != "\r") 
    {
        if (line.empty()) break;
        size_t colon = line.find(':');
        if (colon == string::npos) continue;
        string key = line.substr(0, colon);
        string value = line.substr(colon + 2);
        req.headers[key] = value;
    }

    return true;
}

string HttpParser::buildResponse(int status_code, const string& content, const string& content_type)
{
    string status_msg;
    switch (status_code) 
    {
    case 200: status_msg = "OK"; break;
    case 404: status_msg = "Not Found"; break;
    default: status_msg = "Internal Server Error";
    }

    stringstream response;
    response << "HTTP/1.1 " << status_code << " " << status_msg << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << content.size() << "\r\n";
    response << "Connection: keep-alive\r\n";
    response << "Keep-Alive: timeout=10, max=1000\r\n";
    response << "\r\n";
    response << content;

    return response.str();
}

string HttpParser::readHtmlFile(const string& file_path)
{
    ifstream file(file_path);
    if (!file.is_open()) return "<h1>404 Not Found</h1>";
    
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}