TARGET = lib/mpv_inhibit_gnome.so
SRC_DIR = src
XDG_CONFIG_DIR := $(or $(XDG_CONFIG_HOME),$(HOME)/.config)

LDFLAGS += $(shell pkg-config --libs dbus-1)
CFLAGS += -Wall $(shell pkg-config --cflags dbus-1)

SRCS := $(shell find $(SRC_DIR) -name *.c)
OBJS := $(patsubst src/%.c,build/%.o,$(SRCS))

$(TARGET): $(OBJS)
	-@mkdir -p $(@D)
	gcc $^ -o $@ $(CFLAGS) -shared $(LDFLAGS)

build/%.o: src/%.c
	-@mkdir -p $(@D)
	gcc -c $< -o $@ $(CFLAGS) -fPIC $(LDFLAGS)

define INSTALL_PLUGIN
.PHONY: $(1) $(2)
$(1): $$(TARGET)
	install -Dt "$(3)" $$<

$(2):
	-rm -f "$(3)/$$(notdir $$(TARGET))"
endef

$(eval $(call INSTALL_PLUGIN,install,uninstall,$(XDG_CONFIG_DIR)/mpv/scripts))
$(eval $(call INSTALL_PLUGIN,sys-install,sys-uninstall,/usr/share/mpv/scripts))

.PHONY: clean
clean:
	-rm -rf build
	-rm -f $(TARGET)
