#pragma once
#include <dbus/dbus.h>

typedef struct {
	DBusConnection *connection;
	DBusError *error;
} DBH;

DBH *DBH_init();

DBusMessage *DBH_call(DBH *dbh, const char *name, const char *path,
                      const char *interface, const char *method_name,
                      int first_arg_type, ...);

void DBH_destroy(DBH *dbh);
