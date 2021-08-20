DBUS_FLAGS=$(shell pkg-config --libs --cflags dbus-1)

all: main.o dbus_helper.o gnome_session_manager.o
	mkdir -p bin
	gcc -Wall -g -shared main.o dbus_helper.o gnome_session_manager.o -o bin/mpv_inhibit_gnome.so

main.o: src/main.c
	gcc -Wall -g -fPIC -c src/main.c

dbus_helper.o: src/dbus_helper.c
	gcc -Wall -g -fPIC -c $(DBUS_FLAGS) src/dbus_helper.c

gnome_session_manager.o: src/gnome_session_manager.c
	gcc -Wall -g -fPIC -c $(DBUS_FLAGS) src/gnome_session_manager.c

clean:
	rm -f *.o
	rm -f bin/mpv_inhibit_gnome.so
