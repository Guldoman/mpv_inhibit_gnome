#include "gnome_session_manager.h"
#include "dbus_helper.h"

#include <dbus/dbus.h>
#include <stdlib.h>

struct GSM_s {
	DBH *dbh;
	uint32_t cookie;
};

void GSM_inhibit(GSM *gsm, char *app_id, char *reason, uint32_t flags)
{
	if(gsm->cookie)
	{
		// already inhibited
		GSM_uninhibit(gsm); // force uninhibit
	}

	uint32_t toplevel_xid = 0;

	DBusMessage *result = DBH_call(
	    gsm->dbh, "org.gnome.SessionManager", "/org/gnome/SessionManager",
	    "org.gnome.SessionManager", "Inhibit", DBUS_TYPE_STRING, &app_id,
	    DBUS_TYPE_UINT32, &toplevel_xid, DBUS_TYPE_STRING, &reason,
	    DBUS_TYPE_UINT32, &flags, DBUS_TYPE_INVALID);

	if(result == NULL)
	{
		return; // error
	}
	if(dbus_error_is_set(gsm->dbh->error)
	   || !dbus_message_has_signature(result, "u"))
	{
		dbus_message_unref(result);
		return; // wrong signature
	}

	DBusMessageIter iter;
	dbus_message_iter_init(result, &iter);
	dbus_message_iter_get_basic(&iter, &(gsm->cookie));
	dbus_message_unref(result);
}
void GSM_uninhibit(GSM *gsm)
{
	if(!gsm->cookie)
	{
		return; // already uninhibited
	}

	DBusMessage *result =
	    DBH_call(gsm->dbh, "org.gnome.SessionManager",
	             "/org/gnome/SessionManager", "org.gnome.SessionManager",
	             "Uninhibit", DBUS_TYPE_UINT32, &gsm->cookie, DBUS_TYPE_INVALID);

	if(result == NULL)
	{
		return; // error
	}
	dbus_message_unref(result);
	if(dbus_error_is_set(gsm->dbh->error))
	{
		return;
	}

	gsm->cookie = 0;
}

GSM *GSM_init()
{
	GSM *gsm = (GSM *)malloc(sizeof(GSM));

	gsm->dbh = DBH_init();

	if(gsm->dbh == NULL)
	{
		GSM_destroy(gsm);
		return NULL;
	}

	gsm->cookie = 0;

	return gsm;
}
void GSM_destroy(GSM *gsm)
{
	if(gsm->dbh != NULL)
	{
		GSM_uninhibit(gsm); // force unhinibit
		DBH_destroy(gsm->dbh);
	}
	free(gsm);
}
