TARGET := loq-rgb

SRCDIR := src
OBJDIR := obj
BINDIR := bin

SOURCES := $(shell find $(SRCDIR) -name '*.c')
HEADERS := $(shell find $(SRCDIR) -name '*.h')
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
DEPS    := $(OBJECTS:.o=.d)

CPPFLAGS += $(shell pkg-config --cflags libusb-1.0) \
            -iquote $(SRCDIR)
CFLAGS += -MMD -MP \
          -std=c23 -pedantic -Wall -Wextra \
          -Os
LFFLAGS +=
LDLIBS += $(shell pkg-config --libs libusb-1.0)


all: $(BINDIR)/$(TARGET)

clean:
	@printf '\033[40;38;5;75m << Cleaning >> \033[0m\n'
	@rm -rf $(OBJDIR) $(BINDIR)


PREFIX ?= /usr/local
DESTDIR ?=

install: all
	install -Dm755 $(BINDIR)/$(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)


format: $(SOURCES) $(HEADERS)
	@printf '\033[40;38;5;69m << Formatting >> \033[0m\n'
	clang-format --style=file:.clang_format -i $^

iwyu: $(SOURCES)
	@printf '\033[40;38;5;105m << IWYU check >> \033[0m\n'
	@for f in $(SOURCES); do \
		include-what-you-use $(CPPFLAGS) $(CFLAGS) "$$f" || true; \
	done


$(BINDIR)/$(TARGET): $(OBJECTS)
	@printf '\033[40;38;5;171m << Linking >> \033[0m\n'
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@printf '\033[40;38;5;141m << Compiling >> \033[0m\n'
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: all clean install uninstall format iwyu
