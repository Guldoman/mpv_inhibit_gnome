#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <mpv/client.h>

#include "gnome_session_manager.h"

enum {
	PAUSE = 0,
	IDLE_ACTIVE,
	STOP_SCREENSAVER,
	WINDOW_MINIMIZED,
	MUTE,
	ALBUMART,
	VID,
	AID,
	LAST_FLAG,
};

typedef enum {
	FLAG_INACTIVE = 0,
	FLAG_ACTIVE,
	FLAG_INVALID
} FlagStatus;

#define FLAG_COUNT LAST_FLAG

const struct {
	const char* name;
	int flag_type;
} flag_prop_names[FLAG_COUNT] = {
	{"pause"                         , MPV_FORMAT_FLAG  },
	{"idle-active"                   , MPV_FORMAT_FLAG  },
	{"stop-screensaver"              , MPV_FORMAT_FLAG  },
	{"window-minimized"              , MPV_FORMAT_FLAG  },
	{"mute"                          , MPV_FORMAT_FLAG  },
	{"current-tracks/video/albumart" , MPV_FORMAT_FLAG  },
	{"vid"                           , MPV_FORMAT_INT64 },
	{"aid"                           , MPV_FORMAT_INT64 },
};

typedef struct {
	// handles
	mpv_handle *handle;
	GSM *gsm;

	// mpv props
	bool props[FLAG_COUNT];

	// current status
	GSM_flags inhibition_status;
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
	globals->gsm    = NULL;
	globals->handle = NULL;

	for (unsigned i = 0; i < FLAG_COUNT; i++) {
		globals->props[i] = false;
	}
	globals->inhibition_status = 0;
}


void update_prop(plugin_globals *globals, unsigned prop_index, bool value)
{
	globals->props[prop_index] = value;
	bool* p = globals->props;

	// If the video track is not an album art, the media is surely a video
	bool visually_perceivable = p[VID]
	                        && !p[ALBUMART]
	                        && !p[WINDOW_MINIMIZED];
	// If there is any audio at all
	bool auditory_perceivable = p[AID] && !p[MUTE];
	bool playing_perceivable  = !(p[IDLE_ACTIVE] || p[PAUSE])
	                          && (auditory_perceivable || visually_perceivable);

	bool inhibit_suspend = p[STOP_SCREENSAVER] && playing_perceivable;
	bool inhibit_idle    = inhibit_suspend && visually_perceivable;

	GSM_flags new_status = (inhibit_idle    * GSM_INHIBIT_IDLE)
	                     | (inhibit_suspend * GSM_INHIBIT_SUSPEND);

	if(globals->inhibition_status != new_status)
	{
		globals->inhibition_status = new_status;
		if(new_status)
		{
			show_text(globals->handle,
			          (inhibit_idle ? "Starting inhibit: idle,suspend"
			                        : "Starting inhibit: suspend"));
			GSM_inhibit(globals->gsm, "mpv",
			            inhibit_idle ? "Playing video" : "Playing audio", new_status);
		}
		else
		{
			show_text(globals->handle, "Stopping inhibit");
			GSM_uninhibit(globals->gsm);
		}
	}
}

FlagStatus read_flag_prop(mpv_event_property *prop) {
	switch (prop->format) {
		case MPV_FORMAT_INT64:
			return (*(int64_t *)prop->data) > 0 ? FLAG_ACTIVE : FLAG_INACTIVE;
		case MPV_FORMAT_FLAG:
			return (*(bool *)prop->data) ? FLAG_ACTIVE : FLAG_INACTIVE;
		case MPV_FORMAT_NONE:
			// The property is disabled
			return FLAG_INACTIVE;
		default:
			// Ignore any other format
			return FLAG_INVALID;
	}
}

int mpv_open_cplugin(mpv_handle *handle)
{
	bool done = 0;
	plugin_globals globals = {0};
	globals.handle = handle;

	globals.gsm = GSM_init();
	if(globals.gsm == NULL)
	{
		return -1; // Error while opening dbus
	}

	for(unsigned i = 0; i < FLAG_COUNT; i++)
	{
		mpv_observe_property(globals.handle, 0, flag_prop_names[i].name, flag_prop_names[i].flag_type);
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

				for(unsigned i = 0; i < FLAG_COUNT; i++)
				{
					if(strcmp(prop->name, flag_prop_names[i].name) == 0)
					{
						FlagStatus status = read_flag_prop(prop);
						if (status != FLAG_INVALID) {
							update_prop(&globals, i, status == FLAG_ACTIVE);
						}
						break;
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
