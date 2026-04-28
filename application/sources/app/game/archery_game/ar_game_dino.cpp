#include "ar_game_dino.h"

#include "app_dbg.h"
#include "button.h"
#include "screens_bitmap.h"

extern button_t btn_down;

ar_game_dino_t dino;

void ar_game_dino_reset() {
    dino.y = AR_DINO_GROUND_Y_SCALED - (AR_DINO_H * 10);
    dino.v_y = 0;
    dino.is_jumping = false;
    dino.is_ducking = false;
}

void ar_game_dino_update() {
    dino.is_ducking = (btn_down.state == BUTTON_SW_STATE_PRESSED);

    if (dino.is_jumping) {
        dino.y += dino.v_y;
        dino.v_y += AR_DINO_GRAVITY_SCALED;

        if (dino.y >= (AR_DINO_GROUND_Y_SCALED - (AR_DINO_H * 10))) {
            dino.y = AR_DINO_GROUND_Y_SCALED - (AR_DINO_H * 10);
            dino.is_jumping = false;
            dino.v_y = 0;
        }
    }
}

bool ar_game_dino_hit_test(const ar_game_object_t* obj) {
    int16_t dy = dino.y / 10;
    int16_t dino_hit_y = dy;
    int16_t dino_hit_h = AR_DINO_H;
    int16_t obj_x = obj->x / 10;

    if (dino.is_ducking) {
        dino_hit_y = dy + 6;
        dino_hit_h = 10;
    }

    bool hit_x = (AR_DINO_X + AR_DINO_W - 4 > obj_x) && (AR_DINO_X + 2 < obj_x + obj->w);
    bool hit_y = (dino_hit_y + dino_hit_h > obj->y + 2) && (dino_hit_y + 2 < obj->y + obj->h);

    return hit_x && hit_y;
}

void ar_game_dino_render() {
    const unsigned char* dino_bmp = dino.is_ducking ? bitmap_dino_duck : bitmap_dino;
    view_render.drawBitmap(AR_DINO_X, dino.y / 10, dino_bmp, AR_DINO_W, AR_DINO_H, WHITE);
}

void ar_game_dino_jump() {
    if (!dino.is_jumping) {
        dino.v_y = AR_DINO_JUMP_SCALED;
        dino.is_jumping = true;
    }
}

void ar_game_dino_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case AR_GAME_DINO_SETUP:
    case AR_GAME_DINO_RESET: {
        APP_DBG_SIG("AR_GAME_DINO_RESET\n");
        ar_game_dino_reset();
    }
        break;

    case AR_GAME_DINO_UPDATE: {
        ar_game_dino_update();
    }
        break;

    case AR_GAME_DINO_JUMP: {
        APP_DBG_SIG("AR_GAME_DINO_JUMP\n");
        ar_game_dino_jump();
    }
        break;

    default:
        break;
    }
}
