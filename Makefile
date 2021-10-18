CC = g++
CC_ARM = aarch64-linux-gnu-g++
CFLAGS = -I./include -I./src/ -Wall -Wextra -Werror -O3 -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE -Wformat-security -fno-strict-overflow -fno-delete-null-pointer-checks -static-libstdc++ -static-libgcc
TARGET = fcsServer
SOURCE_FILES = ./src/*.cpp
MOCK_FILES = ./test/mocks/sys/*.cpp ./test/mocks/*.cpp
MAIN_CPP = ./src/main/*.cpp
TEST_EXEC = ./test.x86

.PHONY: all x86 aarch64 test clean
all: clean x86 aarch64

x86:
	$(CC) $(CFLAGS) -o $(TARGET).x86 $(SOURCE_FILES) $(MAIN_CPP)

aarch64:
	$(CC_ARM) $(CFLAGS) -o $(TARGET).aarch64 $(SOURCE_FILES) $(MAIN_CPP)
	cp -f $(TARGET).aarch64 $(TARGET)

test:
	./build_gtest.sh
	$(CC)  $(CFLAGS) -I./test/mocks ./lib/libgtest_main.a ./lib/libgtest.a -o $(TEST_EXEC) $(MOCK_FILES) $(SOURCE_FILES) ./test/*.cpp -pthread
	$(TEST_EXEC)

clean:
	$(RM) *.o
	$(RM) $(TARGET)
	$(RM) $(TARGET).aarch64
	$(RM) $(TARGET).x86
	$(RM) $(TEST_EXEC)
