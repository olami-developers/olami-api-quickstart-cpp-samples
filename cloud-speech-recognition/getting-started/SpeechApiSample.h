/*
    Copyright 2017, VIA Technologies, Inc. & OLAMI Team.
	
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
	
    http://www.apache.org/licenses/LICENSE-2.0
    
	Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>

using namespace std;

struct MemoryStruct {
	char *memory;
	size_t size;
};

struct data {
	char trace_ascii; /* 1 or 0 */
};

class SpeechApiSample {

public:
	static string API_NAME_ASR;

	SpeechApiSample();
	void setAuthorization(string, string);
	void setLocalization(string);
	string getRecognitionResult(string,string);
	string sendAudioFile(string, string, bool, string, bool);

private:
	string getSignMsg(string, long long);
	string getPostData(string, string, string, long long);
	string getBasicQueryString(string,string);
	static size_t writeMemoryCallback(void *, size_t, size_t, void *);
	int saveResponse(const char *);
	string getValueFromResponse(string);
	string getResultFromResponse();
	string trim(const string&);
	void dump(const char *,FILE *, unsigned char *, size_t, char);
	int my_trace(CURL *, curl_infotype,char *, size_t,void *);
	string _apiBaseUrl;
	string _appKey;
	string _appSecret;
	vector<string> _responseHeader;
	string _cookies;
};
