CC ?= gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = $(shell $(PKGCONFIG) --cflags gtk+-3.0 epoxy)
LIBS = $(shell $(PKGCONFIG) --libs gtk+-3.0 epoxy) -lm 
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)
GLIB_COMPILE_SCHEMAS = $(shell $(PKGCONFIG) --variable=glib_compile_schemas gio-2.0)

SRC = glarea-app.c glarea-app-window.c main.c
GEN = glarea-resources.c
BIN = glarea

ALL = $(GEN) $(SRC)
OBJS = $(ALL:.c=.o)

all: $(BIN)

glarea-resources.c: glarea.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=. --generate-dependencies glarea.gresource.xml)
	$(GLIB_COMPILE_RESOURCES) glarea.gresource.xml --target=$@ --sourcedir=. --generate-source

%.o: %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<

$(BIN): $(OBJS)
	$(CC) -o $(@F) $(LIBS) $(OBJS)

clean:
	rm -f $(GEN)
	rm -f $(OBJS)
	rm -f $(BIN)
