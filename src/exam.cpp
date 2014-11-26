//============================================================================
// Name        : exam.cpp
// Author      : netmindms
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <unordered_map>

#include "EdUrlParser.h"

using namespace std;

int main() {
	string s =
			"http://www.google.co.kr:8080/testurl/depth1/depth2/depth3?name=james&id=100";

	// parse and allocate url object.
	EdUrlParser* url = EdUrlParser::parseUrl(s);
	cout << "scheme: " + url->scheme << endl;
	cout << "host name: " + url->hostName << endl;
	cout << "port: " + url->port << endl;
	cout << "path: " + url->path << endl;
	cout << "param: " + url->query << endl;
	cout << "fragment: " + url->fragment << endl;

	// parse path
	vector<string> paths;
	EdUrlParser::parsePath(&paths, url->path);
	for (int i = 0; i < paths.size(); i++) {
		cout << "path part: " + paths[i] << endl;
	}

	// parse query string as key-value list style
	vector<query_kv_t> kvs;
	int num = EdUrlParser::parseKeyValueList(&kvs, url->query);
	for(int i=0;i<num;i++) {
		printf("idx:%d  key: %s, val: %s\n", i, kvs[i].key.c_str(), kvs[i].val.c_str());
	}

	// parse query string as key-value hash map
	unordered_map<string, string> map;
	int mapnum = EdUrlParser::parseKeyValueMap(&map, url->query);
	string name;
	try {
		name = map.at("name");
		printf("key name vale '%s'\n", map["name"].c_str());
	} catch(out_of_range err) {
		printf("### Error: not found...\n");
	};


	if (url != NULL) // ==> make sure to free url object allocated by EdUrlParser
		delete url;

	// test url encoding
	string enc = EdUrlParser::urlEncode("left| |right & ");
	cout << "encoded string: " << enc << endl;
	string dec = EdUrlParser::urlDecode(enc);
	cout << "decoded string: " << dec << endl;

	return 0;
}

