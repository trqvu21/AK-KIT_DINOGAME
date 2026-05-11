#include "ar_game_background.h"

#include <stdlib.h>

#include "app_dbg.h"
#include "screens_bitmap.h"
#include "view_render.h"

ar_game_bg_t ar_game_cloud;

void ar_game_background_reset() {
    ar_game_cloud.x = 120 * 10;
    ar_game_cloud.y = 10;
    ar_game_cloud.w = 16;
    ar_game_cloud.h = 16;
}

void ar_game_background_update() {
    ar_game_cloud.x -= AR_DINO_BG_SPEED_SCALED;
    if (ar_game_cloud.x / 10 < -16) {
        ar_game_cloud.x = (128 + (rand() % 50)) * 10;
    }
}

void ar_game_background_render() {
    view_render.drawBitmap(ar_game_cloud.x / 10,
                           ar_game_cloud.y,
                           bitmap_cloud,
                           ar_game_cloud.w,
                           ar_game_cloud.h,
                           WHITE);
}

void ar_game_background_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case AR_GAME_BACKGROUND_SETUP:
    case AR_GAME_BACKGROUND_RESET: {
        APP_DBG_SIG("AR_GAME_BACKGROUND_RESET\n");
        ar_game_background_reset();
    }
        break;

    case AR_GAME_BACKGROUND_UPDATE: {
        ar_game_background_update();
    }
        break;

    default:
        break;
    }
}
