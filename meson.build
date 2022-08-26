project('mpv_inhibit_gnome', 'c',
  version: '1',
  license: 'MIT',
  default_options: ['warning_level=3', 'prefix=/etc/']
)

# option('debug', type : 'boolean', value : false)
srcs = [
  'src/dbus_helper.c',
  'src/gnome_session_manager.c',
  'src/main.c'
]
incs = []
if not meson.get_compiler('c').has_header('mpv/client.h')
  fs = import('fs')
  mpv_client_h = meson.source_root() / 'include' / 'mpv' / 'client.h'
  if not fs.is_file(mpv_client_h)
    mpv_client_h_dir = fs.parent(mpv_client_h)
    if not fs.is_dir(mpv_client_h_dir)
      run_command('mkdir', '-p', mpv_client_h_dir, check: true)
    endif
    mpv_header_url = 'https://raw.githubusercontent.com/mpv-player/mpv/v0.34.1/libmpv/client.h'
    warning('Downloading mpv/client.h from', mpv_header_url)
    run_command('curl', '--silent', '-o',  mpv_client_h, mpv_header_url, check: true)
  endif
  incs += 'include'
endif

shared_library(
  meson.project_name(),
  srcs,
  install: true,
  override_options: 'b_lundef=false',
  include_directories : incs,
  dependencies : [
    dependency('dbus-1').partial_dependency(compile_args: true, includes: true)
  ],
  name_prefix : '',
  install_dir : 'mpv' / 'scripts'
)

MPV_FLATPAK='io.mpv.Mpv'
run_target('flatpak-override', command: ['flatpak', 'override', '--user', '--talk-name=org.gnome.SessionManager', MPV_FLATPAK])
run_target('flatpak-unoverride', command: ['flatpak', 'override', '--user', '--no-talk-name=org.gnome.SessionManager', MPV_FLATPAK])
