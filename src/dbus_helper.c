#include "dbus_helper.h"
#include <stdlib.h>

bool DBH_method_check(DBH *dbh, const char *name, const char *path,
                      const char *interface, const char *method_name,
                      int first_arg_type, ...)
{
	va_list args;
	va_start(args, first_arg_type);

	DBusMessage *method_call =
	    dbus_message_new_method_call(name, path, interface, method_name);

	dbus_message_append_args_valist(method_call, first_arg_type, args);

	bool result =
	    dbus_message_is_method_call(method_call, interface, method_name);

	dbus_message_unref(method_call);

	return result;
}

DBusMessage *DBH_call(DBH *dbh, const char *name, const char *path,
                      const char *interface, const char *method_name,
                      int first_arg_type, ...)
{
	va_list args;
	va_start(args, first_arg_type);

	DBusMessage *method_call =
	    dbus_message_new_method_call(name, path, interface, method_name);

	dbus_message_append_args_valist(method_call, first_arg_type, args);

	DBusMessage *method_reply = dbus_connection_send_with_reply_and_block(
	    dbh->connection, method_call, 1000, dbh->error);

	dbus_message_unref(method_call);
	if(dbus_error_is_set(dbh->error))
	{
		dbus_error_free(dbh->error);
		if(method_reply != NULL)
		{
			dbus_message_unref(method_reply);
		}
		return NULL;
	}

	return method_reply;
}

DBH *DBH_init()
{
	DBH *dbh = (DBH *)malloc(sizeof(DBH));

	dbh->connection = NULL;
	dbh->error      = (DBusError *)calloc(1, sizeof(DBusError));

	dbus_error_init(dbh->error);
	dbh->connection = dbus_bus_get(DBUS_BUS_SESSION, dbh->error);

	if(dbh->connection == NULL || dbus_error_is_set(dbh->error))
	{
		DBH_destroy(dbh);
		return NULL;
	}

	return dbh;
}

void DBH_destroy(DBH *dbh)
{
	if(dbh->connection != NULL)
	{
		dbus_connection_flush(dbh->connection); // flush remaining messages
		dbus_connection_unref(dbh->connection);
	}
	dbus_error_free(dbh->error);
	free(dbh->error);
	free(dbh);
}
