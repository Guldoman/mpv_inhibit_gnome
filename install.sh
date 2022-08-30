#!/bin/bash
PREFIX=${XDG_CONFIG_HOME:-${HOME}/.config}
TARGET=install
for i in "$@"
do
case $i in
	flatpak)
		PREFIX=~/.var/app/io.mpv.Mpv/config
		;;
	debug)
		SETUP_ARGS= --buildtype=debug
		;;
	*)
		TARGET=${TARGET} $i
		;;
esac
done

[ -d build ] && rm -r build
meson$SETUP_ARGS --prefix=$PREFIX build
ninja --verbose -C build $TARGET
