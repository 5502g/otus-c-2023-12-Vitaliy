all: main.c
	$(CC) $^ $(CFLAGS) -Wall -Wextra -Wpedantic -std=c11 `pkg-config --cflags --libs libcurl json-c`

.PHONY: all
