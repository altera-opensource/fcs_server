CC = g++
CC_ARM = aarch64-linux-gnu-g++
CFLAGS = -Iinclude -Wall -Wextra -Werror -O3 -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE -Wformat-security -fno-strict-overflow -fno-delete-null-pointer-checks

TARGET = fcsServer

all: x86 aarch64

x86: clean
	$(CC) $(CFLAGS) -o $(TARGET).x86 ./src/*.cpp

aarch64: clean
	$(CC_ARM) $(CFLAGS) -o $(TARGET).aarch64 ./src/*.cpp
	cp -f $(TARGET).aarch64 $(TARGET)


clean:
	$(RM) *.o
	$(RM) $(TARGET)
	$(RM) $(TARGET).aarch64
	$(RM) $(TARGET).x86
