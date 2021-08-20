# `mpv_inhibit_gnome`

This mpv plugin prevents screen blanking in GNOME while playing media.

This is needed because neither mpv supports GNOME's inhibition protocol, nor
GNOME supports the standard inhibition protocol
([yet](https://gitlab.gnome.org/GNOME/mutter/-/merge_requests/111)).

## Manually building and installing

The library `libdbus-1` is needed. In Arch Linux it's provided by the package
`dbus`.

Running `make` generates a file called `mpv_inhibit_gnome.so` inside the `bin`
directory.
Copy this file into your mpv scripts directory
(per user: `~/.config/mpv/scripts` or globally: `/usr/share/mpv/scripts/`).
