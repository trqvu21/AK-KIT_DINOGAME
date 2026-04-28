#include "scr_archery_game.h"

uint8_t ar_game_state = 0;
ar_game_setting_t settingsetup;
static uint8_t gameplay_tick_divider = 0;

static void ar_game_setup_modules() {
    ar_game_world_reset();
    ar_game_dino_reset();
    ar_game_objects_reset();
    ar_game_background_reset();
    ar_game_rf_setup();
}

static void ar_game_update_modules() {
    if (ar_game_mp_state == AR_DINO_MP_PLAYING) {
        ar_game_world_update();
        ar_game_dino_update();
        ar_game_background_update();
        ar_game_objects_update();
    }
}

static void render_gameplay() {
    ar_game_world_render_hud();
    ar_game_background_render();
    ar_game_objects_render();
    ar_game_dino_render();
    ar_game_world_render_attack_warning();
}

void view_scr_dino_game() {
    view_render.clear();

    if (ar_game_mp_state == AR_DINO_MP_PLAYING) {
        render_gameplay();
    }
    else {
        ar_game_rf_render_lobby();
    }
}

static void view_wrapper() {
    view_scr_dino_game();
}

view_dynamic_t dyn_view_item_archery_game = { { ITEM_TYPE_DYNAMIC }, view_wrapper };
view_screen_t scr_archery_game = { &dyn_view_item_archery_game, ITEM_NULL, ITEM_NULL, .focus_item = 0 };

void scr_archery_game_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case SCREEN_ENTRY: {
        APP_DBG_SIG("SCREEN_ENTRY\n");
        eeprom_read(EEPROM_SETTING_START_ADDR, (uint8_t*)&settingsetup, sizeof(settingsetup));
        ar_game_state = GAME_PLAY;
        ar_game_mp_state = AR_DINO_MP_WAITING;
        gameplay_tick_divider = 0;
        ar_game_setup_modules();
        timer_set(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK, 10, TIMER_ONE_SHOT);
    }
        break;

    case AR_GAME_TIME_TICK: {
        ar_game_rf_poll();

        if (ar_game_mp_state == AR_DINO_MP_PLAYING) {
            gameplay_tick_divider++;
            if (gameplay_tick_divider >= 2) {
                gameplay_tick_divider = 0;
                ar_game_update_modules();
                view_scr_dino_game();
                view_render.update();
            }
        }
        else {
            gameplay_tick_divider = 0;
        }

        timer_set(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK, 10, TIMER_ONE_SHOT);
        SCREEN_NONE_UPDATE_MASK();
    }
        break;

    case AC_DISPLAY_BUTTON_UP_PRESSED: {
        if (ar_game_mp_state == AR_DINO_MP_PLAYING) {
            ar_game_dino_jump();
        }
        SCREEN_NONE_UPDATE_MASK();
    }
        break;

    case AC_DISPLAY_BUTTON_UP_RELEASED: {
        if (ar_game_mp_state == AR_DINO_MP_WAITING) {
            ar_game_rf_accept();
        }
        else {
            SCREEN_NONE_UPDATE_MASK();
        }
    }
        break;

    case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
        if (ar_game_mp_state == AR_DINO_MP_WAITING) {
            ar_game_rf_ready();
        }
        else {
            SCREEN_NONE_UPDATE_MASK();
        }
    }
        break;

    case AC_DISPLAY_BUTTON_DOWN_PRESSED: {
        SCREEN_NONE_UPDATE_MASK();
    }
        break;

    case AC_DISPLAY_BUTTON_MODE_RELEASED: {
        ar_game_state = GAME_OFF;
        timer_remove_attr(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK);
        SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
    }
        break;

    default:
        SCREEN_NONE_UPDATE_MASK();
        break;
    }
}
