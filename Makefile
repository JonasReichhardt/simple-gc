CC = gcc
CFLAGS = -g -Wall -Wextra -Wpedantic
CFLAGSR = -O3 -Wall -Wextra -Wpedantic -Werror
TARGET = main
BUILD_DIR = build
DEPS = heap.h objects.h type_desc.h heap_util.h


$(TARGET): $(TARGET).c $(DEPS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(TARGET) $(DEPS) $(TARGET).c

release:
	$(CC) $(CFLAGSR) -o $(BUILD_DIR)/$(TARGET) $(DEPS) $(TARGET).c

check: $(TARGET)
	valgrind $(BUILD_DIR)/$(TARGET) --leak-check=full

run: $(TARGET)
	@$(BUILD_DIR)/$(TARGET)

clean:
	rm -f $(TARGET)
