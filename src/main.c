#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <mpv/client.h>

#include "gnome_session_manager.h"

typedef struct {
	mpv_handle *handle;
	GSM *gsm;
} plugin_globals;

void show_text(mpv_handle *handle, const char *text)
{
#ifdef DEBUG
	const char *command[] = {"show-text", text, NULL};
	mpv_command(handle, command);
#endif
}

void begin_inhibit(plugin_globals *globals)
{
	show_text(globals->handle, "Starting inhibit");
	GSM_inhibit(globals->gsm, "mpv", "Media is playing", GSM_INHIBIT_IDLE);
}
void end_inhibit(plugin_globals *globals)
{
	show_text(globals->handle, "Stopping inhibit");
	GSM_uninhibit(globals->gsm);
}

int mpv_open_cplugin(mpv_handle *handle)
{
	bool done = 0;
	plugin_globals globals;
	globals.handle = handle;

	globals.gsm = GSM_init();
	if(globals.gsm == NULL)
	{
		return -1; // error while opening dbus
	}

	mpv_observe_property(handle, 0, "pause", MPV_FORMAT_FLAG);

	mpv_event *last_event    = NULL;
	mpv_event_property *prop = NULL;
	// event loop
	while(!done)
	{
		last_event = mpv_wait_event(handle, -1);

		switch(last_event->event_id)
		{
			case MPV_EVENT_PROPERTY_CHANGE:
				prop = last_event->data;
				if(strcmp(prop->name, "pause") != 0 || prop->format != MPV_FORMAT_FLAG)
					break;

				int pause_status = *(int *)(prop->data);
				if(pause_status == 0)
				{
					begin_inhibit(&globals);
				}
				else
				{
					end_inhibit(&globals);
				}

				break;
			case MPV_EVENT_SHUTDOWN: // quit
				GSM_destroy(globals.gsm);
				done = true;
			default:
				break;
		}
	}
	return 0;
}
