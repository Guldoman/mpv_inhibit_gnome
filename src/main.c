#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <mpv/client.h>

#include "gnome_session_manager.h"

#define GET_FLAG(field, index) (((field) & (1 << (index))) >> (index))
#define SET_FLAG(field, index, value) (\
	(value) ?\
		(field) |= (1 << (index))\
	:\
		((field) &= ~(1 << (index)))\
	)

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

#define props_field uint8_t
static_assert(LAST_FLAG <= sizeof(props_field) * 8, "Increase props_field size");

const struct {
	const char* name;
	int flag_type;
} flag_prop_names[] = {
	{"pause"                         , MPV_FORMAT_FLAG  },
	{"idle-active"                   , MPV_FORMAT_FLAG  },
	{"stop-screensaver"              , MPV_FORMAT_FLAG  },
	{"window-minimized"              , MPV_FORMAT_FLAG  },
	{"mute"                          , MPV_FORMAT_FLAG  },
	{"current-tracks/video/albumart" , MPV_FORMAT_FLAG  },
	{"vid"                           , MPV_FORMAT_INT64 },
	{"aid"                           , MPV_FORMAT_INT64 },
	{NULL                            , MPV_FORMAT_NONE  },
};
static_assert(sizeof(flag_prop_names) / sizeof(flag_prop_names[0]) == LAST_FLAG + 1, "Wrong number of flag_prop_names");

typedef struct {
	// handles
	mpv_handle *handle;
	GSM *gsm;

	// mpv props
	props_field props;

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

	globals->props = 0;
	globals->inhibition_status = 0;
}


void update_prop(plugin_globals *globals, unsigned prop_index, bool value)
{
	SET_FLAG(globals->props, prop_index, value);
	props_field props = globals->props;

	// If the video track is not an album art, the media is surely a video
	bool visually_perceivable = GET_FLAG(props, VID)
	                        && !GET_FLAG(props, ALBUMART)
	                        && !GET_FLAG(props, WINDOW_MINIMIZED);
	// If there is any audio at all
	bool auditory_perceivable = GET_FLAG(props, AID) && !GET_FLAG(props, MUTE);
	bool playing_perceivable  = !(GET_FLAG(props, IDLE_ACTIVE) || GET_FLAG(props, PAUSE))
	                          && (auditory_perceivable || visually_perceivable);

	bool inhibit_suspend = GET_FLAG(props, STOP_SCREENSAVER) && playing_perceivable;
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
	if(!GSM_available(globals.gsm))
	{
		// The dbus interface isn't available,
		// so we're likely not needed
		GSM_destroy(globals.gsm);
		return 0;
	}

	for(unsigned i = 0; flag_prop_names[i].name; i++)
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

				for(unsigned i = 0; flag_prop_names[i].name; i++)
				{
					if(strcmp(prop->name, flag_prop_names[i].name) == 0)
					{
						bool value = false;
						switch (prop->format) {
							case MPV_FORMAT_INT64:
								value = *(int64_t *)prop->data > 0;
								break;
							case MPV_FORMAT_FLAG:
								value = *(bool *)prop->data;
								break;
							case MPV_FORMAT_NONE:
								// The property is disabled, so keep value as false
								break;
							default:
								// Ignore any other format
								goto done_update_prop;
								break;
						}
						update_prop(&globals, i, value);
done_update_prop:
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
