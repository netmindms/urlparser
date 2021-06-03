/*
    The MIT License (MIT)
    
    Copyright (c) 2014 Blake Kim, https://github.com/netmindms/urlparser
    Copyright (c) 2021 James Johnston, https://github.com/idolpx/urlparser
    All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef EDURLPARSER_H_
#define EDURLPARSER_H_

#include <unordered_map>
#include <tuple>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

typedef struct {
    string key;
    string val;
} query_kv_t;

typedef int (*__kv_callback)(void* list, string k, string v);



class EdUrlParser 
{
private:
    EdUrlParser();

    void parse();
    void string_toupper(std::string &s);

public:
    virtual ~EdUrlParser();

    void parsePath(string pathstr);
    static int parsePath(vector<string>* folders, string pathstr);
    static EdUrlParser* parseUrl(string urlstr);

    static string urlEncode(string s);	
    static string urlDecode(string str);
    static void toHex(char *desthex, char c);
    static char toChar(const char* hex);
    
    static size_t parseKeyValueMap(unordered_map<string, string> *kvmap, string str, bool strict=true);
    static size_t parseKeyValueList(vector< query_kv_t > *kvmap, string rawstr, bool strict=true);
    static size_t parseKeyValue(string rawstr, __kv_callback kvcb, void* obj, bool strict);

    string url;
    string root;
    string base;
    string scheme;
    string username;
    string password;
    string hostname;
    string port;
    string path;
    string filename;
    string extension;
    string query;
    string fragment;
};

#endif /* EDURLPARSER_H_ */
