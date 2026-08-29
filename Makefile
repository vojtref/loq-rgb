MAKEFLAGS += --jobs=$(shell nproc 2>/dev/null || echo 1)
MAKEFLAGS += --output-sync=target

TARGET := $(shell basename $(CURDIR))

PREFIX ?= /usr/local
DESTDIR ?=

SRCDIR := src
OBJDIR := obj
BINDIR := bin

SOURCES := $(shell find $(SRCDIR) -name '*.c')
HEADERS := $(shell find $(SRCDIR) -name '*.h')
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
DEPS    := $(OBJECTS:.o=.d)

CC := clang
CFLAGS := -iquote $(SRCDIR) \
          -MMD -MP \
          -std=c23 -pedantic -Wall -Wextra \
          -Os
LFLAGS :=
LDLIBS := usb-1.0

all: $(BINDIR)/$(TARGET)

install: all
	install -Dm755 $(BINDIR)/$(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	@printf '\033[40;38;5;75m << Cleaning >> \033[0m\n'
	@rm -rf $(OBJDIR) $(BINDIR)

format: $(SOURCES) $(HEADERS)
	@printf '\033[40;38;5;69m << Formatting >> \033[0m\n'
	clang-format --style=file:.clang_format -i $^

iwyu: $(SOURCES) | format
	@printf '\033[40;38;5;105m << IWYU check >> \033[0m\n'
	@for f in $(SOURCES); do \
		include-what-you-use $(CFLAGS) "$$f" || true; \
	done

$(BINDIR)/$(TARGET): $(OBJECTS)
	@printf '\033[40;38;5;171m << Linking >> \033[0m\n'
	@mkdir -p $(@D)
	$(CC) $(LFLAGS) $^ $(addprefix -l,$(LDLIBS)) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | iwyu
	@printf '\033[40;38;5;141m << Compiling >> \033[0m\n'
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: all clean format iwyu
