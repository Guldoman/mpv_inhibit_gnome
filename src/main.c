#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <mpv/client.h>

#include "gnome_session_manager.h"

typedef struct {
	// handles
	mpv_handle *handle;
	GSM *gsm;

	// config options
	bool enable;

	// mpv status
	bool pause;
	bool idle_active;
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
	if(!globals->enable)
		return;

	show_text(globals->handle, "Starting inhibit");
	GSM_inhibit(globals->gsm, "mpv", "Media is playing", GSM_INHIBIT_IDLE);
}
void end_inhibit(plugin_globals *globals)
{
	if(!globals->enable)
		return;

	show_text(globals->handle, "Stopping inhibit");
	GSM_uninhibit(globals->gsm);
}

void init_globals(plugin_globals *globals)
{
	// handles
	globals->gsm    = NULL;
	globals->handle = NULL;

	// config options
	globals->enable = false;

	// mpv status
	globals->pause       = true;
	globals->idle_active = false;
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

	// If false disable inhibition
	mpv_observe_property(globals.handle, 0, "stop-screensaver", MPV_FORMAT_FLAG);

	// Returns true if no file is loaded
	// We need to check this property because `pause` may be false while no
	// media is being played and `--idle` is specified
	mpv_observe_property(globals.handle, 0, "idle-active", MPV_FORMAT_FLAG);

	// True if paused
	mpv_observe_property(globals.handle, 0, "pause", MPV_FORMAT_FLAG);

	mpv_event *last_event    = NULL;
	mpv_event_property *prop = NULL;
	// Event loop
	while(!done)
	{
		last_event = mpv_wait_event(handle, -1);

		switch(last_event->event_id)
		{
			case MPV_EVENT_PROPERTY_CHANGE:
				prop = last_event->data;

				if(prop->format == MPV_FORMAT_FLAG)
				{
					if(strcmp(prop->name, "pause") == 0)
					{
						globals.pause = *(int *)(prop->data);

						// If `idle_active` is true, we are not really paused
						if(!globals.idle_active)
						{
							if(!globals.pause)
							{
								begin_inhibit(&globals);
							}
							else
							{
								end_inhibit(&globals);
							}
						}
					}
					else if(strcmp(prop->name, "idle-active") == 0)
					{
						int old_idle_active = globals.idle_active;
						globals.idle_active = *(int *)(prop->data);

						if(!old_idle_active && globals.idle_active)
						{
							// The player became idle-active
							if(!globals.pause) // Avoid ending inhibit uselessly
							{
								end_inhibit(&globals);
							}
						}
						else if(old_idle_active && !globals.idle_active)
						{
							// The player stopped being idle-active
							// so we follow the pause property
							// We were already uninhibited, so only check if
							// playing
							if(!globals.pause)
							{
								begin_inhibit(&globals);
							}
						}
					}
					else if(strcmp(prop->name, "stop-screensaver") == 0)
					{
						int old_enable = globals.enable;
						globals.enable = *(int *)(prop->data);

						if(old_enable && !globals.enable)
						{
							// We got disabled, stop inhibition in any case
							end_inhibit(&globals);
						}
						else if(!old_enable && globals.enable)
						{
							// We got enabled
							if(!globals.pause && !globals.idle_active)
							{
								begin_inhibit(&globals);
							}
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
