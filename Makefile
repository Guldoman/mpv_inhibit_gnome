TARGET = lib/mpv_inhibit_gnome.so
SRC_DIR = src

C_FLAGS = -Wall -g -fPIC -I./include/ $(shell pkg-config --libs --cflags dbus-1)

SRCS := $(shell find $(SRC_DIR) -name *.c)
OBJS := $(patsubst src/%.c,build/%.o,$(SRCS))
MPV_INCL := include/mpv/client.h

$(TARGET): $(OBJS)
	-@mkdir -p $(@D)
	gcc -Wall -g -shared $^ -o $@

$(MPV_INCL):
	-@mkdir -p $(@D)
	curl --silent -o "$@" https://raw.githubusercontent.com/mpv-player/mpv/v0.34.1/libmpv/$(@F)

build/%.o: src/%.c $(MPV_INCL)
	-@mkdir -p $(@D)
	gcc -c $(C_FLAGS) $< -o $@


define INSTALL_PLUGIN
.PHONY: $(1) $(2)
$(1): $$(TARGET)
	install -t "$(3)" $$<

$(2):
	-rm -f "$(3)/$$(notdir $$(TARGET))"
endef

$(eval $(call INSTALL_PLUGIN,install,uninstall,$(HOME)/.config/mpv/scripts))
$(eval $(call INSTALL_PLUGIN,sys-install,sys-uninstall,/usr/share/mpv/scripts))

MPV_FLATPAK=io.mpv.Mpv
$(eval $(call INSTALL_PLUGIN,flatpak-install,flatpak-uninstall,$(HOME)/.var/app/$(MPV_FLATPAK)/config/mpv/scripts))

.PHONY: flatpakoverride
flatpakoverride:
	flatpak override --user --talk-name=org.gnome.SessionManager $(MPV_FLATPAK)

.PHONY: flatpakunoverride
flatpakunoverride:
	flatpak override --user --no-talk-name=org.gnome.SessionManager $(MPV_FLATPAK)


.PHONY: clean
clean:
	-rm -rf build
	-rm -f $(TARGET)
