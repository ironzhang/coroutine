TARGET = test

all: $(TARGET)

$(TARGET): coroutine.c main.c
	gcc -g -Wall -o $@ $^

clean:
	rm $(TARGET)

