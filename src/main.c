#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <mpv/client.h>

#include "gnome_session_manager.h"

#define FLAG_PROP_COUNT 8

typedef struct __attribute__((packed)) {
	bool pause;
	bool idle_active;
	bool stop_screensaver;
	bool window_minimized;
	bool mute;
	bool albumart;
	bool vid;
	bool aid;
} PropsFlags;

typedef union {
	PropsFlags flags;
	bool values[FLAG_PROP_COUNT];
} Props;


typedef struct {
	// handles
	mpv_handle *handle;
	GSM *gsm;

	// active inhibition flags
	uint32_t flags;

	// mpv props
	Props props;
} plugin_globals;

void show_text(mpv_handle *handle, const char *text)
{
#ifdef DEBUG
	const char *command[] = {"show-text", text, NULL};
	mpv_command(handle, command);
#endif
}

void init_globals(plugin_globals *globals)
{
	// handles
	globals->gsm    = NULL;
	globals->handle = NULL;

	globals->flags = 0;
	// mpv props
	globals->props.flags = (PropsFlags){
	    // True if paused
	    .pause = true,
	    // If true, no file is loaded
	    // We need to check this property because `pause` may be false while no
	    // media is being played and `--idle` is specified
	    .idle_active = false,
	    // If false, disable inhibition
	    .stop_screensaver = false,
	    // If playing audio minimized, only inhibit suspend
	    .window_minimized = false,
	    // No inhibition if no audio and no video
	    .mute = false,
	    .albumart  = false,
	    .vid  = false,
	    .aid  = false};
}


void update_prop(plugin_globals *globals, unsigned prop_index, bool value)
{
	globals->props.values[prop_index] = value;
	PropsFlags props                  = globals->props.flags;

	bool visually_perceivable = props.vid && !props.window_minimized && !props.albumart;
	bool auditory_perceivable = props.aid && !props.mute;
	bool playing_perceivable  = !(props.idle_active || props.pause)
	                           && (auditory_perceivable || visually_perceivable);

	bool inhibit_suspend = props.stop_screensaver && playing_perceivable;
	bool inhibit_idle    = inhibit_suspend && visually_perceivable;

	uint32_t new_flags = (inhibit_idle * GSM_INHIBIT_IDLE)
	                     | (inhibit_suspend * GSM_INHIBIT_SUSPEND);

	if(globals->flags != new_flags)
	{
		globals->flags = new_flags;
		if(new_flags)
		{
			show_text(globals->handle,
			          (inhibit_idle ? "Starting inhibit: idle,suspend"
			                        : "Starting inhibit: suspend"));
			GSM_inhibit(globals->gsm, "mpv",
			            inhibit_idle ? "Playing video" : "Playing audio", new_flags);
		}
		else
		{
			show_text(globals->handle, "Stopping inhibit");
			GSM_uninhibit(globals->gsm);
		}
	}
}

int mpv_open_cplugin(mpv_handle *handle)
{
	bool done = 0;
	plugin_globals globals;
	init_globals(&globals);
	globals.handle = handle;

	globals.gsm = GSM_init();
	if(globals.gsm == NULL)
	{
		return -1; // Error while opening dbus
	}

	const char *const flag_prop_names[FLAG_PROP_COUNT] = {
	    "pause", "idle-active", "stop-screensaver", "window-minimized", "mute",
	    "current-tracks/video/albumart", "vid", "aid",
	};
	for(unsigned i = 0; i < FLAG_PROP_COUNT; i++)
	{
		mpv_observe_property(globals.handle, 0, flag_prop_names[i],
		                     i < FLAG_PROP_COUNT - 2 ? MPV_FORMAT_FLAG
		                                             : MPV_FORMAT_INT64);
	}

	mpv_event *event         = NULL;
	mpv_event_property *prop = NULL;
	// Event loop
	while(!done)
	{
		event = mpv_wait_event(handle, -1);

		switch(event->event_id)
		{
			case MPV_EVENT_PROPERTY_CHANGE:
				prop = event->data;

				if(prop->format == MPV_FORMAT_FLAG || prop->format == MPV_FORMAT_INT64)
				{
					for(unsigned i = 0; i < FLAG_PROP_COUNT; i++)
					{
						if(strcmp(prop->name, flag_prop_names[i]) == 0)
						{
							update_prop(&globals, i, *(bool *)(prop->data));
							break;
						}
					}
				}
				break;
			case MPV_EVENT_SHUTDOWN: // quit
				// Will automatically uninhibit
				GSM_destroy(globals.gsm);
				done = true;
			default:
				break;
		}
	}
	return 0;
}
