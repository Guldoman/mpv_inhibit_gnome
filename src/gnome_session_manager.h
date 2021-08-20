#pragma once
#include <stdint.h>

#define GSM_INHIBIT_LOGOUT 1
#define GSM_INHIBIT_USER_SWITCH 2
#define GSM_INHIBIT_SUSPEND 4
#define GSM_INHIBIT_IDLE 8

typedef struct GSM_s GSM;

GSM *GSM_init();

void GSM_inhibit(GSM *gsm, const char *app_id, const char *reason,
                 uint32_t flags);
void GSM_uninhibit(GSM *gsm);

void GSM_destroy(GSM *gsm);
