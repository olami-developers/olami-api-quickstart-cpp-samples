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

#include <algorithm>
#include "SpeechApiSample.h"
#include <unistd.h>

int main(int argc, const char* argv[]) {

	if (argc < 5) {
		cout << endl<<endl << "[Error] Missing argv! " << endl << "Usage:" << endl
			<< " - argv[1]: api_url" << endl
			<< " - argv[2]: your_app_key" << endl
			<< " - argv[3]: your_app_secret" << endl
			<< " - argv[4]: your_audio_file" << endl
			<< " - argv[5]: compress_flag=[0|1]" << endl
			<< endl << endl;
		return 0;
	}

	string url = argv[1];
	string appKey = argv[2];
	string appSecret = argv[3];
	string filePath = argv[4];
	string str_compressed = argv[5];
	bool compressed = str_compressed.compare("1") == 0;

	SpeechApiSample speechApi;
	speechApi.setLocalization(url);
	speechApi.setAuthorization(appKey, appSecret);

	// Start sending audio file for recognition
	cout <<  "\n----- Test Speech API, seq=nli,seg -----\n" << endl
		<< "\nSend audio file..." << endl;
	string responseString =  speechApi.sendAudioFile(SpeechApiSample::API_NAME_ASR,
			"nli,seg", true, filePath, compressed);
	cout << "\n\nResult:\n\n" << responseString << endl;

	// Try to get recognition result if uploaded successfully.
	// We just check the state by a lazy way :P , you should do it by JSON.
	transform(responseString.begin(), responseString.end(), responseString.begin(), ::tolower);
	if (responseString.find("error") == string::npos) {
		cout << "\n----- Get Recognition Result -----\n" << endl;
		usleep(1000000); /* sleep 1 second */
		// Try to get result until the end of the recognition is complete
		while (true) {
			responseString = speechApi.getRecognitionResult(
					SpeechApiSample::API_NAME_ASR, "nli,seg");
			cout << "\n\nResult:\n\n" + responseString <<  endl;
			// Well, check by lazy way...again :P , do it by JSON please.
			transform(responseString.begin(), responseString.end(), responseString.begin(), ::tolower);
			if (responseString.find("\"final\":true") == string::npos) {
				cout << "The recognition is not yet complete." << endl;
				if (responseString.find("error") != string::npos) break;
				usleep(2000000); /* sleep 2 second */
			} else {
				break;
			}
		}
	}

	return 0;
}
