CC = g++
CC_ARM = aarch64-linux-gnu-g++
CFLAGS = -std=c++17 -I./include -I./src/ -Wall -Wextra -Werror -O3 -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE -Wformat-security -fno-strict-overflow -fno-delete-null-pointer-checks -static-libstdc++ -static-libgcc
CFLAGS_DEBUG = -std=c++17 -I./include -I./src/ -g -O0 -DDEBUG -static-libstdc++ -static-libgcc
TARGET = fcsServer
SOURCE_FILES = ./src/*.cpp
MOCK_FILES = ./test/mocks/sys/*.cpp ./test/mocks/*.cpp
MAIN_CPP = ./src/main/*.cpp
TEST_EXEC = ./test.x86

.PHONY: build all x86 x86sim aarch64 test clean

build: clean x86 aarch64

all: clean x86 aarch64 test

ifeq ($(DEBUG), 1)
        CFLAGS=$(CFLAGS_DEBUG)
endif

x86sim:
	./downloadSpdmSim.sh
	$(CC) -DSPDM_SIM -I./spdmSim/inc/ $(CFLAGS) -Wl,-rpath,'$$ORIGIN'/spdmSim/lib -I./test/mocks -o $(TARGET).x86sim $(MOCK_FILES) $(SOURCE_FILES) $(MAIN_CPP) -ldl

x86:
	$(CC) $(CFLAGS) -o $(TARGET).x86 $(SOURCE_FILES) $(MAIN_CPP) -ldl

aarch64:
	$(CC_ARM) $(CFLAGS) -o $(TARGET).aarch64 $(SOURCE_FILES) $(MAIN_CPP)
	cp -f $(TARGET).aarch64 $(TARGET)

test:
	./build_gtest.sh
	$(CC) -I./spdmSim/inc/ $(CFLAGS) -I./test/mocks ./lib/libgtest_main.a ./lib/libgtest.a -o $(TEST_EXEC) $(MOCK_FILES) $(SOURCE_FILES) ./test/*.cpp -pthread -ldl
	$(TEST_EXEC)

clean:
	$(RM) *.o
	$(RM) $(TARGET)
	$(RM) $(TARGET).aarch64
	$(RM) $(TARGET).x86
	$(RM) $(TARGET).x86sim
	$(RM) $(TEST_EXEC)
