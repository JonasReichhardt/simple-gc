CC = gcc
CFLAGS = -g
CFLAGSR = -Wall -Wextra -Wpedantic -Werror
TARGET = main
DEPS = heap


$(TARGET): $(TARGET).c $(DEPS).h
	$(CC) $(CFLAGS) -o build/$(TARGET) $(DEPS).h $(TARGET).c

release:
	$(CC) $(CFLAGS) -o $(TARGET) $(DEPS).h $(TARGET).c

run: $(TARGET)
	@build/$(TARGET)

clean:
	rm -f $(TARGET)
