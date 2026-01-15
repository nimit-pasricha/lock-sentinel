CC = gcc
# -fPIC: Position Independent Code. Required for shared libraries because
# the library can be loaded at any memory address.
# -shared: Tells the linker to produce a .so file, not an executable.
CFLAGS = -Wall -fPIC -shared
# -ldl: Links the 'libdl' library, which has dlsym().
LDFLAGS = -ldl

# changed name cuz libguard is already a thing
all: libsentinel.so deadlock

libsentinel.so: sentinel.c
	$(CC) $(CFLAGS) sentinel.c -o libsentinel.so $(LDFLAGS)

deadlock: deadlock.c
	$(CC) deadlock.c -o deadlock

clean:
	rm -f libsentinel.so deadlock