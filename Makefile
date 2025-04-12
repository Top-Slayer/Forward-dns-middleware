CC = clang
CFLAGS = -Iinclude -Wall
LDFLAGS = -lcjson
SRC = main.c $(wildcard src/*.c)
BUILD = build
OBJ = $(patsubst %.c, $(BUILD)/%.o, $(SRC))
TARGET = app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(OBJ:.o=.d)

clean:
	rm -rf $(BUILD) $(TARGET)

compressjs:
	if command -v uglifyjs >/dev/null 2>&1; then \
		uglifyjs ./public/connect-domain.js -c -m > ./public/conn-dom.js; \
		echo "Compressed successful"; \
	else \
		npm install -g uglifyjs; \
		echo "Installed successful"; \
	fi

.PHONY: all clean