#ifndef __AR_GAME_WORLD_H__
#define __AR_GAME_WORLD_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "app.h"
#include "ar_game_common.h"

void ar_game_world_update();
void ar_game_world_reset();
void ar_game_world_render_hud();
void ar_game_world_render_attack_warning();
void ar_game_world_handle(ak_msg_t* msg);

#endif //__AR_GAME_WORLD_H__
