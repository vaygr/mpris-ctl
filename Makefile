BIN_NAME := mpris-ctl
CC ?= clang
LIBS = dbus-1
COMPILE_FLAGS = -std=c99 -Wpedantic -Wall -Wextra
LINK_FLAGS =
RCOMPILE_FLAGS = -DNDEBUG
DCOMPILE_FLAGS = -g -DDEBUG -O1
RLINK_FLAGS =
DLINK_FLAGS =

SOURCES = src/main.c
DESTDIR = /
INSTALL_PREFIX = usr/local
MAN_DIR = share/man

ifneq ($(LIBS),)
	CFLAGS += $(shell pkg-config --cflags $(LIBS))
	LDFLAGS += $(shell pkg-config --libs $(LIBS))
endif

ifeq ($(shell git describe > /dev/null 2>&1 ; echo $$?), 0)
	VERSION := $(shell git describe --tags --long --dirty=-git --always )
endif
ifneq ($(VERSION), )
	override CFLAGS := $(CFLAGS) -DVERSION_HASH=\"$(VERSION)\"
endif

.PHONY: all debug check memory undefined check_memory check_undefined check_leak run release debug clean install uninstall

all: debug

ifneq ($(CC),clang)
check_memory:
	$(error Only clang supports memory sanitizer check, current compiler "$(CC)")

check: check_leak check_undefined
else
check_memory:
	$(MAKE) memory clean

check: check_leak check_undefined check_memory
endif

leak: export CFLAGS := $(CFLAGS) $(COMPILE_FLAGS) $(DCOMPILE_FLAGS) -fsanitize=address
leak:
	$(MAKE) BIN_NAME=mpris-ctl-leak
	$(MAKE) BIN_NAME=mpris-ctl-leak run

memory: export CFLAGS := $(CFLAGS) $(COMPILE_FLAGS) $(DCOMPILE_FLAGS) -fsanitize=memory -fsanitize-blacklist=sanitize-blacklist.txt -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer
memory:
	$(MAKE) BIN_NAME=mpris-ctl-memory
	$(MAKE) BIN_NAME=mpris-ctl-memory run

undefined: export CFLAGS := $(CFLAGS) $(COMPILE_FLAGS) $(DCOMPILE_FLAGS) -fsanitize=undefined
undefined:
	$(MAKE) BIN_NAME=mpris-ctl-undef
	$(MAKE) BIN_NAME=mpris-ctl-undef run

check_leak:
	$(MAKE) leak clean

check_undefined:
	$(MAKE) undefined clean

run: $(BIN_NAME)
	./$(BIN_NAME) info %full || test $$? -eq 1

release: export CFLAGS := $(CFLAGS) $(COMPILE_FLAGS) $(RCOMPILE_FLAGS)
release: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RLINK_FLAGS)
debug: export CFLAGS := $(CFLAGS) $(COMPILE_FLAGS) $(DCOMPILE_FLAGS)
debug: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(DLINK_FLAGS)

$(BIN_NAME).1: $(BIN_NAME).1.scd
	scdoc < $< >$@

release: $(BIN_NAME)

debug: $(BIN_NAME)

clean:
	$(RM) $(BIN_NAME) $(BIN_NAME)-*
	$(RM) $(BIN_NAME).1

install: $(BIN_NAME) $(BIN_NAME).1
	install $(BIN_NAME) $(DESTDIR)$(INSTALL_PREFIX)/bin
	install -m 644 $(BIN_NAME).1 $(DESTDIR)$(INSTALL_PREFIX)/$(MAN_DIR)/man1

uninstall:
	$(RM) $(DESTDIR)$(INSTALL_PREFIX)/bin/$(BIN_NAME)
	$(RM) $(DESTDIR)$(INSTALL_PREFIX)/$(MAN_DIR)/man1/$(BIN_NAME).1

$(BIN_NAME): $(SOURCES) src/*.h
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) $(LDFLAGS) -o$(BIN_NAME)
