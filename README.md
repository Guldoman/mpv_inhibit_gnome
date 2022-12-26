# `mpv_inhibit_gnome`

[![AUR version](https://img.shields.io/aur/version/mpv_inhibit_gnome)](https://aur.archlinux.org/packages/mpv_inhibit_gnome)

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

This plugin has been integrated into the
[Flatpak release of mpv](https://flathub.org/apps/details/io.mpv.Mpv),
so you should already be good to go.

If this plugin was manually installed before the integration,
uninstall it to avoid conflicts by deleting
`~/.var/app/io.mpv.Mpv/config/mpv/scripts/mpv_inhibit_gnome.so`.
