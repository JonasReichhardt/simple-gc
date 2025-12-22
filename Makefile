CC = gcc
CFLAGS = -g
CFLAGSR = -Wall -Wextra -Wpedantic -Werror
TARGET = main
DEPS = heap.h objects.h type_desc.h


$(TARGET): $(TARGET).c $(DEPS)
	$(CC) $(CFLAGS) -o build/$(TARGET) $(DEPS) $(TARGET).c

release:
	$(CC) $(CFLAGS) -o $(TARGET) $(DEPS) $(TARGET).c

run: $(TARGET)
	@build/$(TARGET)

clean:
	rm -f $(TARGET)
