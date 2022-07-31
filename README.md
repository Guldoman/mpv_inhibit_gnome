# `mpv_inhibit_gnome`

This mpv plugin prevents screen blanking in GNOME while playing media.

This is needed because neither mpv supports GNOME's inhibition protocol, nor
GNOME supports the standard inhibition protocol
([yet](https://gitlab.gnome.org/GNOME/mutter/-/merge_requests/111)).

## Install
Depends on library: `libdbus-1` (Arch: `dbus`, Fedora: `dbus-devel`)
Install copies `lib/mpv_inhibit_gnome.so` to mpv scripts directory:
```bash
# install for user: ~/.config/mpv/scripts
make install
# or install for flatpak: ~/.var/app/io.mpv.Mpv/config/mpv/scripts
# flatpak override --user --talk-name=org.gnome.SessionManager io.mpv.Mpv
make flatpak-install flatpakoverride
# or install for system: /usr/share/mpv/scripts
sudo make sys-install
```
