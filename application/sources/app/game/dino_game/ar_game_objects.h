#ifndef __AR_GAME_OBJECTS_H__
#define __AR_GAME_OBJECTS_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "app.h"
#include "ar_game_common.h"

extern ar_game_object_t ar_game_objects[AR_DINO_OBJECT_COUNT];

void ar_game_objects_reset();
void ar_game_objects_update();
void ar_game_objects_render();
void ar_game_objects_handle(ak_msg_t* msg);

#endif
