CC = gcc
CFLAGS = -g -Wall -Wextra -Wpedantic
CFLAGSR = -O3 -Wall -Wextra -Wpedantic -Werror
TARGET = main
DEPS = heap.h objects.h type_desc.h heap_util.h


$(TARGET): $(TARGET).c $(DEPS)
	$(CC) $(CFLAGS) -o build/$(TARGET) $(DEPS) $(TARGET).c

release:
	$(CC) $(CFLAGSR) -o build/$(TARGET) $(DEPS) $(TARGET).c

run: $(TARGET)
	@build/$(TARGET)

clean:
	rm -f $(TARGET)
