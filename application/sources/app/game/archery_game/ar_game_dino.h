#ifndef __AR_GAME_DINO_H__
#define __AR_GAME_DINO_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "app.h"
#include "view_render.h"
#include "ar_game_common.h"

extern ar_game_dino_t dino;

void ar_game_dino_render();
void ar_game_dino_update();
void ar_game_dino_jump();
void ar_game_dino_reset();
bool ar_game_dino_hit_test(const ar_game_object_t* obj);
void ar_game_dino_handle(ak_msg_t* msg);

#endif //__AR_GAME_DINO_H__
