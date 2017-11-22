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
#include <cstdio>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include "SpeechApiSample.h"
#include "lib/md5.h"

string SpeechApiSample::API_NAME_ASR = "asr";

SpeechApiSample::SpeechApiSample() {
}

/**
 * Setup your authorization information to access OLAMI services.
 *
 * @param appKey the AppKey you got from OLAMI developer console.
 * @param appSecret the AppSecret you from OLAMI developer console.
 */
void SpeechApiSample::setAuthorization(string _appKey, string _appSecret) {
	this->_appKey = _appKey;
	this->_appSecret = _appSecret;
}

/**
 * Setup localization to select service area, this is related to different
 * server URLs or languages, etc.
 *
 * @param language the language type.
 */
void SpeechApiSample::setLocalization(string _apiBaseUrl) {
	this->_apiBaseUrl = _apiBaseUrl;
}

void SpeechApiSample::dump(const char *text,
          FILE *stream, unsigned char *ptr, size_t size,
          char nohex) {
	size_t i;
	size_t c;

	unsigned int width=0x10;

	if(nohex)
		/* without the hex output, we can fit more on screen */
		width = 0x40;

	fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n", text, (long)size, (long)size);

	for(i=0; i<size; i+= width) {

		fprintf(stream, "%4.4lx: ", (long)i);

		if(!nohex) {
			/* hex not disabled, show it */
			for(c = 0; c < width; c++)
				if(i+c < size)
					fprintf(stream, "%02x ", ptr[i+c]);
				else
					fputs("   ", stream);
		}

		for(c = 0; (c < width) && (i+c < size); c++) {
			/* check for 0D0A; if found, skip past and start a new line of output */
			if(nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
				i+=(c+2-width);
				break;
			}
			fprintf(stream, "%c", (ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');
			/* check again for 0D0A, to avoid an extra \n if it's at width */
			if(nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
				i+=(c+3-width);
				break;
			}
		}
		fputc('\n', stream); /* newline */
	}
	fflush(stream);
}

int SpeechApiSample::my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp) {
	struct data *config = (struct data *)userp;
	const char *text;
	(void)handle; /* prevent compiler warning */

	switch(type) {
	case CURLINFO_TEXT:
		fprintf(stderr, "== Info: %s", data);
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
//	case CURLINFO_DATA_OUT:
//		text = "=> Send data";
//		break;
//	case CURLINFO_SSL_DATA_OUT:
//		text = "=> Send SSL data";
//		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
//	case CURLINFO_SSL_DATA_IN:
//		text = "<= Recv SSL data";
//		break;
	}

	dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
	return 0;
}

/**
 * Send an audio file to speech recognition service.
 *
 * @param apiName the API name for 'api=xxx' HTTP parameter.
 * @param seqValue the value of 'seq' for 'seq=xxx' HTTP parameter.
 * @param finished TRUE to finish upload or FALSE to continue upload.
 * @param filePath the path of the audio file you want to upload.
 * @param compressed TRUE if the audio file is a Speex audio.
 */
string SpeechApiSample::sendAudioFile(string apiName, string seqValue,
		bool finished, string filePath, bool compressed) {

	// Read the input audio file
	ifstream inFile;
	size_t size = 0;
	char* oData = 0;
	inFile.open(filePath.c_str(), ios::in|ios::binary|ios::ate);

	if(inFile.is_open()) {
		inFile.seekg(0, ios::end);
		size = inFile.tellg();
		inFile.seekg(0, ios::beg);

		oData = new char[size+1];
		inFile.read(oData, size);
		oData[size] = '\0';

		cout << "file.size=" << size << endl;
	} else {
		string retrunMsg = "[ERROR] File not found!";
		return retrunMsg;
	}

	string postData = getBasicQueryString(apiName,seqValue);
	postData += "&compress=";
	postData += compressed? "1" : "0";
	postData += "&stop=";
	postData += finished? "1" : "0";

	string result = "";
	long http_code = -1;

	CURL *curl;
	CURLcode res;

	struct MemoryStruct chunk;
	chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	/* for debugfunction use*/
	struct data config;
	config.trace_ascii = 1; /* enable ascii tracing */

	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);
	/* get a curl handle */
	curl = curl_easy_init();
	if(curl) {
		/* for debug use */
//		res = curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
//		res = curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
//		/* set VERBOSE to 1 to see more log from libcurl */
//		res = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* specify URL to get */
		//cause we need to send wave file at the same time, so some parameter need combine with url
		res = curl_easy_setopt(curl, CURLOPT_URL, (_apiBaseUrl+"?"+postData).c_str());
		/* start cookie engine */
		res = curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
		/* let response with header to get value of Set-Cookie */
		res = curl_easy_setopt(curl, CURLOPT_HEADER, 1);

		/* add header */
		struct curl_slist *headers = NULL;
		headers = curl_slist_append (headers, "Expect: 100-continue");
		headers = curl_slist_append (headers, "Connection: Keep-Alive");
		headers = curl_slist_append (headers, "Content-type: multipart/form-data");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		/* post sound file */
		struct curl_httppost *post=NULL;
		struct curl_httppost *last=NULL;
		curl_formadd(&post, &last,
					 CURLFORM_COPYNAME, "sound",
					 CURLFORM_BUFFER, "filePath.c_str()",
					 CURLFORM_BUFFERPTR, oData,
					 CURLFORM_BUFFERLENGTH, size,
					 CURLFORM_END);
		/* Set the form info */
		res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

		/* send all data to this function  */
		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SpeechApiSample::writeMemoryCallback);
		/* we pass our 'chunk' struct to the callback function */
		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		/* some servers don't like requests that are made without a user-agent
		 field, so we provide one */
		res = curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
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
			saveResponse(chunk.memory);
		}

		// Now you can check the status here.
		cout << "Sending 'POST' request to URL : " << _apiBaseUrl << endl;
		cout << "Post parameters : " << postData << endl;
		cout << "Response Code : " << http_code << endl;

		// Get cookie
		_cookies = getValueFromResponse("Set-Cookie:");
		cout << "Cookies : "<< _cookies << endl;
		// Get the response
		result = getResultFromResponse();

		/* free headers*/
		curl_slist_free_all(headers);
		/* free post */
		curl_formfree(post);
		/* cleanup curl stuff */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();


	return result;
}

/**
 * save response headers split use newline symbol.
 */
int SpeechApiSample::saveResponse(const char *response) {
	stringstream ss(response);
	string s;

	while (getline(ss, s, '\n')) {
		if(s[0] != '\r'&& s[0] != '\n') {
			_responseHeader.push_back(s);
		}
	}
	return _responseHeader.size();
}

/**
 * return response header which has the key.
 */
string SpeechApiSample::getValueFromResponse(string headerKey) {

	size_t found;
	size_t i=0;
	// find where headerKey is
	for(;i<_responseHeader.size();i++) {
		found = _responseHeader[i].find(headerKey);
		if (found != string::npos) {
			//cout << " found at: " << found << '\n';
			break;
		}
	}

	// get value of headerKey
	if(i<_responseHeader.size()) {
		return trim(_responseHeader[i].substr(headerKey.length()));
	} else {
		cout << "ERROR: no value found with " << headerKey << endl;
		return "";
	}

}

/**
 * trim string: remove empty character in head and tail
 */
string SpeechApiSample::trim(const string& str)
{
	size_t first = str.find_first_not_of(' ');
	if (string::npos == first) {
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

/**
 * get real result: pure JSON format, usually at last one of repsonse headers
 */
string SpeechApiSample::getResultFromResponse() {
	if( !_responseHeader.empty())
		return trim(_responseHeader[_responseHeader.size()-1]);
	else
		return "";
}

/**
 * Get the speech recognition result for the audio you sent.
 *
 * @param apiName the API name for 'api=xxx' HTTP parameter.
 * @param seqValue the value of 'seq' for 'seq=xxx' HTTP parameter.
 */
string SpeechApiSample::getRecognitionResult(string apiName, string seqValue) {
	string queryData = getBasicQueryString(apiName,seqValue) + "&stop=1";
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
		/* set VERBOSE to 1 to see more log from libcurl */
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* add cookie to request header */
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers,("Cookie:"+_cookies).c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		/* specify URL to get */
		curl_easy_setopt(curl, CURLOPT_URL, (_apiBaseUrl+"?"+queryData).c_str());
		/* send all data to this function  */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SpeechApiSample::writeMemoryCallback);
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
		cout << "Sending 'GET' request to URL : " << _apiBaseUrl << endl;
		cout << "Query String : " << queryData << endl;
		cout << "Response Code : " << http_code << endl;

		curl_slist_free_all(headers); /* free custom header list */
		/* cleanup curl stuff */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();

	return result;
}

// Allocate Memory space to put response data here.
size_t SpeechApiSample::writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
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

/**
 * Generate and get a basic HTTP query string
 *
 * @param apiName the API name for 'api=xxx' HTTP parameter.
 * @param seqValue the value of 'seq' for 'seq=xxx' HTTP parameter.
 */
string SpeechApiSample::getBasicQueryString(string apiName, string seqValue) {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	long long timestamp = tp.tv_sec;
	timestamp *= 1000;
	
	string signMsg = getSignMsg(apiName,timestamp);
	string postData = getPostData(apiName,seqValue,signMsg,timestamp);
	return postData;
}

// Prepare message to generate an MD5 digest.
string SpeechApiSample::getSignMsg(string apiName, long long timestamp) {
	if(timestamp == NULL) {
		struct timeval tp;
		gettimeofday(&tp, NULL);
		timestamp = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	}

	stringstream timestr;
	// timestamp transfer to stringstream
	timestr << timestamp;
	string msg = "";
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

// Assemble all the HTTP parameters you want to send
string SpeechApiSample::getPostData(string apiName, string seqValue, string signMsg, long long timestamp) {
	if(timestamp == NULL) {
		struct timeval tp;
		gettimeofday(&tp, NULL);
		timestamp = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	}
	stringstream timestr;
	// timestamp transfer to stringstream
	timestr << timestamp;
	string postData = "_from=cpp";
	postData += "&appkey="+ _appKey;
	postData += "&api="+ apiName;
	postData += "&timestamp="+ timestr.str();
	postData += "&sign="+ signMsg;
	postData += "&seq="+ seqValue;

	return postData;
}
