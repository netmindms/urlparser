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

#include "EdUrlParser.h"

#define CHECK_LEN_END(POS, LEN) if(POS>=LEN) {_url_errorno=100;goto __PARSE_END;}
#define WALK_SP(POS, LEN, BUF) for(;POS<LEN && BUF[POS]==' ';POS++)
#define WALK_UNTIL(POS, LEN, BUF, DELC) for(;POS<LEN && BUF[POS]!=DELC;POS++)
#define WALK_UNTIL2(POS, LEN, BUF, DELI1, DELI2) for(;POS<LEN && BUF[POS]!=DELI1 && BUF[POS]!=DELI2 ;POS++)
#define WALK_UNTIL3(POS, LEN, BUF, DELI1, DELI2, DELI3) for(;POS<LEN && BUF[POS]!=DELI1 && BUF[POS]!=DELI2 && BUF[POS]!=DELI3;POS++)
#define CHECK_REMAIN_END(POS, LEN, REQ_LEN) if(LEN-POS < REQ_LEN) {_url_errorno=100; goto __PARSE_END; }
#define WALK_CHAR(POS, BUF, DELI) if(BUF[POS++] != DELI) goto __PARSE_END

int __kv_callback_map(void* list, string k, string v);
int __kv_callback_vec(void* list, string k, string v);

EdUrlParser::EdUrlParser() {
}

EdUrlParser::~EdUrlParser() {
}

string EdUrlParser::urlEncode(string s) 
{
    const char *ptr = s.c_str();
    string enc;
    char c;
    char phex[3] = { '%' };
    for (size_t i = 0; i < s.size(); i++) {
        c = ptr[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '*'
                || c == '.') {
            enc.push_back(c);
        } else if (c == ' ') {
            enc.push_back('+');
        } else {
            toHex(phex + 1, c);
            enc.append(phex, 0, 3);
        }
    }
    return enc;
}

string EdUrlParser::urlDecode(string str) 
{
    int _url_errorno = 0;
    size_t pos = 0, per = 0;
    size_t len = str.size();
    const char* buf = str.c_str();
    string decstr;
    _url_errorno = 0;
    for (per = pos = 0;;) {
        WALK_UNTIL2(pos, len, buf, '%', '+');
        decstr.append(buf, per, pos - per);
        if (pos >= len)
            goto __PARSE_END;
        if (buf[pos] == '%') {
            CHECK_REMAIN_END(pos, len, 3);
            try {
                char c = EdUrlParser::toChar(buf + pos + 1);
                decstr.push_back(c);
                pos += 3;
                per = pos;
            } catch (int err) {
            	_url_errorno = err;
            	goto __PARSE_END;
            }
            if (pos >= len)
                goto __PARSE_END;
        } else if (buf[pos] == '+') {
            decstr.push_back(' ');
            pos++;
            per = pos;
        }
    }
    __PARSE_END: if (_url_errorno != 0)
        return "";
    return decstr;

}

void EdUrlParser::toHex(char* desthex, char c) 
{
    static char hextable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
            '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    desthex[0] = hextable[c >> 4];
    desthex[1] = hextable[c & 0x0f];
}

char EdUrlParser::toChar(const char* hex) 
{
    unsigned char nible[2];
    unsigned char c, base;
    for (int i = 0; i < 2; i++) {
        c = hex[i];
        if (c >= '0' && c <= '9') {
            base = '0';
        } else if (c >= 'A' && c <= 'F') {
            base = 'A' - 10;
        } else if (c >= 'a' && c <= 'f') {
            base = 'a' - 10;
        } else {
            //throw 200;
        }
        nible[i] = c - base;
    }
    return ((nible[0] << 4) | nible[1]);
}


void EdUrlParser::parse() 
{
    int _url_errorno = 0;
    const char *str = url.c_str();

    size_t pos, len;
    size_t scheme_pos, user_pos, pass_pos, host_pos, port_pos, path_pos, param_pos, tag_pos;
    pos = 0;
    len = url.size();
    WALK_SP(pos, len, str); // remove preceding spaces.
    if (str[pos] == '/') {
        goto __PARSE_HOST;
    }

    // start protocol scheme
    scheme_pos = pos;
    WALK_UNTIL(pos, len, str, ':');
    CHECK_LEN_END(pos, len);
    scheme = url.substr(scheme_pos, pos - scheme_pos);
    CHECK_REMAIN_END(pos, len, 3);
    WALK_CHAR(pos, str, ':');
    WALK_CHAR(pos, str, '/');
    WALK_CHAR(pos, str, '/');

    // start username / password
    user_pos = pos;
    WALK_UNTIL(pos, len, str, '@');
    if (pos == len)
    {
        pos = user_pos;
        goto __PARSE_HOST;
    }
    host_pos = pos;
    pos = user_pos;
    WALK_UNTIL(pos, len, str, ':');
    if (pos > host_pos)
    {
        // Username Only
        pos = user_pos;
        WALK_UNTIL(pos, len, str, '@');
        username = url.substr(user_pos, pos - user_pos);
        WALK_CHAR(pos, str, '@');
    }
    else
    {
        // Username & Password
        username = url.substr(user_pos, pos - user_pos);
        WALK_CHAR(pos, str, ':');
        pass_pos = pos;
        WALK_UNTIL(pos, len, str, '@');
        password = url.substr(pass_pos, pos - pass_pos);
        WALK_CHAR(pos, str, '@');
    }

    // start host address
    __PARSE_HOST:
    host_pos = pos;
    WALK_UNTIL3(pos, len, str, ':', '/', '?');
    if (pos < len) {
        hostname = url.substr(host_pos, pos - host_pos);
        if (str[pos] == ':')
            goto __PARSE_PORT;
        if (str[pos] == '/')
            goto __PARSE_PATH;
        if (str[pos] == '?')
            goto __PARSE_PARAM;
    } else {
        hostname = url.substr(host_pos, pos - host_pos);
    }

    __PARSE_PORT:
    WALK_CHAR(pos, str, ':');
    port_pos = pos;
    WALK_UNTIL2(pos, len, str, '/', '?');
    port = url.substr(port_pos, pos - port_pos);
    CHECK_LEN_END(pos, len);
    if (str[pos] == '?')
        goto __PARSE_PARAM;

    __PARSE_PATH: path_pos = pos;
    WALK_UNTIL(pos, len, str, '?');
    path = url.substr(path_pos, pos - path_pos);
    CHECK_LEN_END(pos, len);

    __PARSE_PARAM:
    WALK_CHAR(pos, str, '?');
    param_pos = pos;
    WALK_UNTIL(pos, len, str, '#');
    query = url.substr(param_pos, pos - param_pos);
    CHECK_LEN_END(pos, len);

    // start parsing fragment
    WALK_CHAR(pos, str, '#');
    tag_pos = pos;
    fragment = url.substr(tag_pos, len - tag_pos);

    __PARSE_END:
        parsePath(path);

        // set root URL
        root = scheme + "://" + hostname;
        if ( port.size() )
            root += ':' + port;
        base = root;
        root += '/';

        // set base URL
        base += path;
        
        return;
}

int EdUrlParser::parsePath(vector<string>* folders, string pathstr) 
{
    int _url_errorno = 0;
    int path_pos = 0;
    size_t pos = 0;
    size_t len = pathstr.size();
    const char* str = pathstr.c_str();
    string name;
    for (pos = 0;;) {
        WALK_CHAR(pos, str, '/');
        path_pos = pos;
        CHECK_LEN_END(pos, len);
        WALK_UNTIL(pos, len, str, '/');
        name = pathstr.substr(path_pos, pos - path_pos);
        folders->push_back(name);
    }
    __PARSE_END: return folders->size();
}

void EdUrlParser::parsePath(string pathstr) 
{
    size_t pos = 0;

    // Path
    pos = pathstr.find_last_of('/');
    path = pathstr.substr(0, pos) + '/';

    // File
    filename = pathstr.substr(pos + 1);

    // Extension
    pos = filename.find_last_of('.');
    if ( pos < filename.size() )
    {
        extension = filename.substr(pos + 1);
        string_toupper(extension);
    }
}

EdUrlParser* EdUrlParser::parseUrl(string urlstr) 
{
    EdUrlParser *url = new EdUrlParser;
    url->url = urlstr;
    url->parse();
    return url;
}

size_t EdUrlParser::parseKeyValueMap(unordered_map<string, string> *kvmap, string rawstr, bool strict) {
    return parseKeyValue(rawstr, __kv_callback_map, kvmap, strict);
}

size_t EdUrlParser::parseKeyValueList(vector< query_kv_t > *kvvec, string rawstr, bool strict) {
    return parseKeyValue(rawstr, __kv_callback_vec, kvvec, strict);
}

size_t EdUrlParser::parseKeyValue(string rawstr, __kv_callback kvcb, void* obj, bool strict) {

    int _url_errorno = 0;
    const char *str = rawstr.c_str();
    size_t pos, len, item_len;
    pos = 0;
    len = rawstr.size();

    string key, val;
    size_t key_pos;
    WALK_SP(pos, len, str);
    CHECK_LEN_END(pos, len);
    key_pos = pos;
    item_len = 0;
    for(;;) {
        WALK_UNTIL2(pos, len, str, '=', '&');
        if(pos >= len || str[pos] == '&') {
            // Be careful for boundary check error to be caused. !!!
            // *** Do not access str[] any more in this block. !!!

            val = rawstr.substr(key_pos, pos-key_pos);

            if(strict == true) {
                if(key.empty() == false && val.empty()==false) {
                    kvcb(obj, key, val);
                    item_len++;
                }
            } else if(!(key.empty()==true && val.empty()==true)){
                kvcb(obj, key, val);
                item_len++;
            }

            key.clear();val.clear();
            if(pos >= len) goto __PARSE_END;
            pos++;
            key_pos = pos;
        }
        else if(str[pos] == '=') {
            key = rawstr.substr(key_pos, pos-key_pos);
            pos++;
            key_pos = pos;
        }
    }
    __PARSE_END:
    if(_url_errorno != 0 )
        return -1;
    return item_len;
}


int __kv_callback_map(void* list, string k, string v) {
    auto *map = (unordered_map<string, string>*)list;
    (*map)[k] = v;
    return map->size();
}

int __kv_callback_vec(void* list, string k, string v) {
    auto *vec = (vector<query_kv_t>*)list;
    query_kv_t t ={k, v};
    vec->push_back(t);
    return vec->size();
}

// convert to uppercase (in place)
void EdUrlParser::string_toupper(string &s)
{
    //std::transform(s.begin(), s.end(), s.begin(),
    //               (unsigned char c) { return std::toupper(c); });

    transform(s.begin(), s.end(), s.begin(), ::toupper);
}
