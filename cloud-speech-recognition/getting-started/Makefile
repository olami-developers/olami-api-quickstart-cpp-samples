#    Copyright 2017, VIA Technologies, Inc. & OLAMI Team.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

CPP=g++
CFLAGS=
CFILES=$(wildcard *.c)
CPPFILES=$(wildcard *.cpp lib/*.cpp)
OBJS=$(CPPFILES:%.cpp=%.o)
LIBDIR=/usr/include
LIB=-lcurl -lpthread


all: asrapi_test

asrapi_test: $(OBJS)
	$(CPP) $(CFLAGS) -L$(LIBDIR) -o asrapi_test $(OBJS) $(LIB)

.cpp.o:
	$(CPP) -c -o $@ $<

clean:
	rm -rf asrapi_test $(wildcard *.o lib/*.o)


