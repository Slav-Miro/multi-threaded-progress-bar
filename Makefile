CC=clang
CFLAGS=-Wall -Wextra -g
SRC=main.c
TARGET=multithreaded_progress

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lpthread

clean:
	rm -f $(TARGET)