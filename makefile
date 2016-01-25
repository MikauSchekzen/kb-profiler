CC=mingw32-g++.exe
# CFLAGS=-I.
# DEPS = interception.h

profiler:
	$(CC) -L.\x86 -L.\rapidjson -LD:.\x86 -o Profiler_Test.exe profiler.cpp src\utils.c x86\interception.lib rapidjson\rapidjson.h x86\winmm.lib src\binding.cpp src\keymap.cpp -std=c++11

detector:
	$(CC) -L.\x86 -L.\src -o HWID_Detector.exe detector.cpp src\utils.c x64\interception.lib
