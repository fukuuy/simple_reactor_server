#pragma once
#include <string>
#include <unordered_map>

using namespace std;

class HttpParser
{
public:
    struct HttpRequest 
    {
        //请求方法
        string method;  
        //请求路径
        string path;
        //HTTP版本         
        string version;
        //请求头     
        unordered_map<string, string> headers;
        //请求体
        string body;         
    };

    //解析请求
    static bool parseRequest(const string& raw_data, HttpRequest& req);
    //构建请求
    static string buildResponse(int status_code, const string& content,
        const string& content_type = "text/html; charset=utf-8");
    //读取网页
    static string readHtmlFile(const string& file_path);
};