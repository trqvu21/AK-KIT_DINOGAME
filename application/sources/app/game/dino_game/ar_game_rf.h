#ifndef __AR_GAME_RF_H__
#define __AR_GAME_RF_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "app.h"
#include "ar_game_common.h"

void ar_game_rf_setup();
void ar_game_rf_poll();
void ar_game_rf_ready();
void ar_game_rf_accept();
void ar_game_rf_reject();
void ar_game_rf_start_solo();
void ar_game_rf_render_lobby();
void ar_game_rf_handle(ak_msg_t* msg);

#endif
