PREFIX = /usr/local
CC = cc
CFLAGS = -O2 -I/usr/local/include
LDFLAGS = -L/usr/local/lib
LIBS = -largon2 -lcrypto
TARGET = age-keygen-deterministic
SRCS = age-keygen-deterministic.c bech32.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
