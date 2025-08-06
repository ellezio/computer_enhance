OUT_DIR := result
OUT_BIN := sim86

INPUT_FILE_PATH ?=

build:
	mkdir -p $(OUT_DIR)
	gcc -o $(OUT_DIR)/$(OUT_BIN) main.c

decode:
	$(MAKE) build
	$(OUT_DIR)/$(OUT_BIN) $(INPUT_FILE_PATH)

execute:
	$(MAKE) build
	$(OUT_DIR)/$(OUT_BIN) --exec $(INPUT_FILE_PATH)
