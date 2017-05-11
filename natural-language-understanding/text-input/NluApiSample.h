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

using namespace std;

struct MemoryStruct {
	char *memory;
	size_t size;
};

class NluApiSample {

public:
	static string API_NAME_SEG;
	static string API_NAME_NLI;

	NluApiSample();
	void setAuthorization(string, string);
	void setLocalization(string);
	string getRecognitionResult(string,string);

private:
	string getSignMsg(string, time_t);
	string getPostData(string, string, string, time_t);
	static size_t writeMemoryCallback(void *, size_t, size_t, void *);
	string _apiBaseUrl;
	string _appKey;
	string _appSecret;
};
