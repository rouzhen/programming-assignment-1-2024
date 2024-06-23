CC = gcc
SRC_DIR = ./source/system_programs
BIN_DIR = ./bin
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BIN_DIR)/%)
MAIN_SRC = ./source/shell.c
MAIN_EXEC = cseshell

# Special rule for main executable
all: $(OBJECTS) $(MAIN_EXEC)

$(BIN_DIR)/%: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $< -o $@

$(MAIN_EXEC): $(MAIN_SRC)
	$(CC) $< -o $@

clean:
	rm -f $(OBJECTS) $(MAIN_EXEC)
