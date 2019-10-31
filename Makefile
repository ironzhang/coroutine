TARGET = test

.PHONY: clean

all: $(TARGET)

$(TARGET): coroutine.c main.c
	gcc -g -Wall -o $@ $^

clean:
	rm -f $(TARGET)

