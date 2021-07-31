#pragma once
#include <dbus/dbus.h>

typedef struct {
	DBusConnection *connection;
	DBusError *error;
} DBH;

DBH *DBH_init();

DBusMessage *DBH_call(DBH *dbh, char *name, char *path, char *interface,
                      char *method_name, int first_arg_type, ...);

void DBH_destroy(DBH *dbh);
