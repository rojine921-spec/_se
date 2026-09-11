CC ?= gcc
CFLAGS = -Wall -Wextra -Iinclude -std=c99 -O2
LDFLAGS = -lws2_32

ifneq ($(shell uname || echo Windows),Linux)
    # Windows: MinGW static link avoids runtime DLL issues
    LDFLAGS += -static
endif

SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

TARGET = $(BINDIR)/mycurl$(EXEEXT)

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

.PHONY: all clean debug

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo Build complete: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

debug:
	$(MAKE) CFLAGS="-Wall -Wextra -Iinclude -std=c99 -g -O0"

clean:
	rm -rf $(OBJDIR) $(BINDIR)