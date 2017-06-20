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
#include <cstdlib>
#include <sstream>
#include <curl/curl.h>
#include <sys/time.h>

#include "NluApiSample.h"
#include "lib/md5.h"

string NluApiSample::API_NAME_SEG = "seg";
string NluApiSample::API_NAME_NLI = "nli";

NluApiSample::NluApiSample() {
}

/**
 * Setup your authorization information to access OLAMI services.
 *
 * @param appKey the AppKey you got from OLAMI developer console.
 * @param appSecret the AppSecret you from OLAMI developer console.
 */
void NluApiSample::setAuthorization(string appKey, string appSecret) {
	_appKey = appKey;
	_appSecret = appSecret;
}

/**
 * Setup localization to select service area, this is related to different
 * server URLs or languages, etc.
 *
 * @param language the language type.
 */
void NluApiSample::setLocalization(string apiBaseUrl) {
	_apiBaseUrl = apiBaseUrl;
}

/**
 * Get the NLU recognition result for your input text.
 *
 * @param inputText the text you want to recognize.
 */
string NluApiSample::getRecognitionResult(string apiName, string inputText) {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int timestamp = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	
	string signMsg = this->getSignMsg(apiName,timestamp);
	string postData = this->getPostData(apiName,inputText,signMsg,timestamp);
	string result = "";
	long http_code = -1;

	CURL *curl;
	CURLcode res;
	struct MemoryStruct chunk;

	chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

	/* get a curl handle */
	curl = curl_easy_init();
	if(curl) {
		/* specify URL to get */
		curl_easy_setopt(curl, CURLOPT_URL, this->_apiBaseUrl.c_str());

		/* Now specify the POST data */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

		/* send all data to this function  */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NluApiSample::writeMemoryCallback);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		/* some servers don't like requests that are made without a user-agent
		 field, so we provide one */
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		/* get it! */
		res = curl_easy_perform(curl);
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);

		/* check for errors */
		if(res != CURLE_OK) {
			cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
		}
		else {
			/*
			 * Now, our chunk.memory points to a memory block that is chunk.size
			 * bytes big and contains the remote file.
			 *
			 * Do something nice with it!
			 */
			printf("%lu bytes retrieved\n", (long)chunk.size);
			result = chunk.memory;
		}

		// Now you can check the status here.
		cout << "Sending 'POST' request to URL : " << this->_apiBaseUrl << endl;
		cout << "Post parameters : " << postData << endl;
		cout << "Response Code : " << http_code << endl;

		/* cleanup curl stuff */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();

	return result;
}

// Allocate Memory space to put response data here.
size_t NluApiSample::writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
	/* out of memory! */
		cout <<"not enough memory (realloc returned NULL)" << endl;
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

// Prepare message to generate an MD5 digest.
string NluApiSample::getSignMsg(string apiName, time_t timestamp) {
	if(timestamp == NULL) {
		timestamp = time(NULL);
	}

	stringstream timestr;
	// timestamp transfer to stringstream
	timestr << timestamp;
	string msg = "";
	msg = "";
	msg += _appSecret;
	msg += "api=";
	msg += apiName;
	msg += "appkey=";
	msg += _appKey;
	msg += "timestamp=";
	msg += timestr.str();
	msg += _appSecret;
	// Generate MD5 digest.
	return md5(msg);
}

// Request NLU service by HTTP POST
string NluApiSample::getPostData(string apiName, string inputText, string signMsg, time_t timestamp) {
	if(timestamp == NULL) {
		timestamp = time(NULL);
	}
	stringstream timestr;
	// timestamp transfer to stringstream
	timestr << timestamp;
	string postData = "appkey="+ _appKey;
	postData += "&api="+ apiName;
	postData += "&timestamp="+ timestr.str();
	postData += "&sign="+ signMsg;
	if(apiName.compare(API_NAME_SEG) == 0) {
		postData += "&rq="+inputText;
	} else if(apiName.compare(API_NAME_NLI) == 0){
		postData += "&rq={\"data\":{\"input_type\":1,\"text\":\""+ inputText +"\"},\"data_type\":\"stt\"}";
	}

	return postData;
}
