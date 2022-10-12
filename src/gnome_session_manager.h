/*
 * This allows talking to the GNOME Session Manager
 * for inhibition purposes only.
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>

#define GSM_INHIBIT_LOGOUT 1
#define GSM_INHIBIT_USER_SWITCH 2
#define GSM_INHIBIT_SUSPEND 4
#define GSM_INHIBIT_IDLE 8

#define GSM_flags uint32_t

typedef struct GSM_s GSM;

GSM *GSM_init();

/*
 * Check if inhibition via the GNOME interface is available.
 */
bool GSM_available(GSM *gsm);

/*
 * Ask the Session Manager to inhibit what is specified by `flags`.
 * - `app_id` is the name of the application that is requiring the inhibition.
 * - `reason` is the justification for the inhibition.
 */
void GSM_inhibit(GSM *gsm, const char *app_id, const char *reason,
                 GSM_flags flags);
void GSM_uninhibit(GSM *gsm);

void GSM_destroy(GSM *gsm);
