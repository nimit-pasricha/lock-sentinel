CC = gcc
# -fPIC: Position Independent Code. Required for shared libraries because
# the library can be loaded at any memory address.
# -shared: Tells the linker to produce a .so file, not an executable.
CFLAGS = -Wall -fPIC -shared
# -ldl: Links the 'libdl' library, which has dlsym().
LDFLAGS = -ldl

# changed name cuz libguard is already a thing
all: libsentinel.so

libsentinel.so: sentinel.c
	$(CC) $(CFLAGS) -o libsentinel.so sentinel.c $(LDFLAGS)

clean:
	rm -f libsentinel.so