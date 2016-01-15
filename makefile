CC=mingw32-g++
# CFLAGS=-I.
# DEPS = interception.h

profiler:
	$(CC) -L.\x86 -L.\rapidjson -LD:.\x86 -o Profiler_Test.exe profiler.cpp utils.c x86\interception.lib rapidjson\rapidjson.h x86\winmm.lib

# profiler.o: profiler.cpp interception.h
# 	$(CC) -c -o profiler.o profiler.cpp -L ./x64 -l interception
