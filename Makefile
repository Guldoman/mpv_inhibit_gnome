TARGET = lib/mpv_inhibit_gnome.so
SRC_DIR = src
XDG_CONFIG_DIR := $(or $(XDG_CONFIG_HOME),$(HOME)/.config)

C_FLAGS = -Wall -g -fPIC $(shell pkg-config --libs --cflags dbus-1)

SRCS := $(shell find $(SRC_DIR) -name *.c)
OBJS := $(patsubst src/%.c,build/%.o,$(SRCS))

$(TARGET): $(OBJS)
	-@mkdir -p $(@D)
	gcc -Wall -g -shared $^ -o $@

build/%.o: src/%.c
	-@mkdir -p $(@D)
	gcc -c $(C_FLAGS) $< -o $@

define INSTALL_PLUGIN
.PHONY: $(1) $(2)
$(1): $$(TARGET)
	install -t "$(3)" $$<

$(2):
	-rm -f "$(3)/$$(notdir $$(TARGET))"
endef

$(eval $(call INSTALL_PLUGIN,install,uninstall,$(XDG_CONFIG_DIR)/mpv/scripts))
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
