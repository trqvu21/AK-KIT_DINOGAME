#ifndef __AR_GAME_BACKGROUND_H__
#define __AR_GAME_BACKGROUND_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "app.h"
#include "ar_game_common.h"

extern ar_game_bg_t ar_game_cloud;

void ar_game_background_update();
void ar_game_background_reset();
void ar_game_background_render();
void ar_game_background_handle(ak_msg_t* msg);

#endif //__AR_GAME_BACKGROUND_H__
