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

#include "NluApiSample.h"

int main(int argc, const char* argv[]) {

	if (argc < 4) {
		cout << endl<<endl << "[Error] Missing argv! " << endl << "Usage:" << endl
			<< " - argv[1]: api_url" << endl
			<< " - argv[2]: your_app_key" << endl
			<< " - argv[3]: your_app_secret" << endl
			<< " - argv[4]: your_text_input" << endl
			<< endl << endl;
		return 0;
	}

	string url = argv[1];
	string appKey = argv[2];
	string appSecret = argv[3];
	string inputText = argv[4];

	NluApiSample nluApi;
	nluApi.setLocalization(url);
	nluApi.setAuthorization(appKey, appSecret);

	cout << endl
		<< "---------- Test NLU API, api=seg ----------" << endl
		<< endl << "Result:" << endl << endl
		<< nluApi.getRecognitionResult(NluApiSample::API_NAME_SEG, inputText) << endl;

	cout << endl
		<< "---------- Test NLU API, api=nli ----------" << endl
		<< endl << "Result:" << endl << endl
		<< nluApi.getRecognitionResult(NluApiSample::API_NAME_NLI, inputText) << endl;

	return 0;
}
