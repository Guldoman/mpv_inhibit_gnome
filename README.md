# `mpv_inhibit_gnome`

This mpv plugin prevents screen blanking in GNOME while playing media.

This is needed because neither mpv supports GNOME's inhibition protocol, nor
GNOME supports the standard inhibition protocol
([yet](https://gitlab.gnome.org/GNOME/mutter/-/merge_requests/111)).

## Installing

You can find the latest release [here](https://github.com/Guldoman/mpv_inhibit_gnome/releases).
Download `mpv_inhibit_gnome.so` and put it in your `mpv` scripts directory
(by default it's `~/.config/mpv/scripts`).

## Manually building and installing

To build, the `libdbus-1` library is needed
(Arch: `dbus`, Fedora: `dbus-devel`, Ubuntu: `libdbus-1-dev`),
as well as the header files for `mpv`
(Arch: `mpv`, Fedora: `mpv-libs-devel`, Ubuntu: `libmpv-dev`).

To build run:
```bash
make
```
This will generate the plugin in `lib/mpv_inhibit_gnome.so`.

To install in the default per-user location `~/.config/mpv/scripts`
either copy it there or run:
```bash
make install
```

To install in the default system-wide location `/usr/share/mpv/scripts` run:
```bash
sudo make sys-install
```

## Flatpak support

TL;DR:
```bash
make flatpak-install flatpakoverride
```
and you're good to go.

This will install the plugin in `~/.var/app/io.mpv.Mpv/config/mpv/scripts`,
and will punch a hole in the Flatpak sandbox to allow `mpv` to talk
to the D-Bus address that GNOME uses for inhibiting screen blanking.

You can manually allow `mpv` to talk to the D-Bus address by running:
```bash
flatpak override --user --talk-name=org.gnome.SessionManager io.mpv.Mpv
```
It's also possible to use
[Flatseal](https://flathub.org/apps/details/com.github.tchx84.Flatseal)
to do that.
