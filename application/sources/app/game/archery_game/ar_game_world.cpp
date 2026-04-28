#include "ar_game_world.h"

#include "app_dbg.h"
#include "buzzer.h"
#include "screens.h"
#include "view_render.h"

uint32_t ar_game_score = 0;
uint8_t ar_game_mp_state = AR_DINO_MP_WAITING;
uint8_t ar_game_frame_skip = 0;
int32_t ar_game_current_speed = AR_DINO_BASE_SPEED_SCALED;
uint16_t ar_game_attack_timer = 0;
static uint8_t speed_level = 0;
static uint8_t speed_up_notice_timer = 0;

static int32_t world_base_speed() {
    int32_t speed = AR_DINO_BASE_SPEED_SCALED;
    speed += (settingsetup.num_arrow - 1) * AR_DINO_SETTING_SPEED_STEP;
    speed += (ar_game_score / AR_DINO_SCORE_STEP) * AR_DINO_SPEED_STEP;

    return speed > AR_DINO_MAX_BASE_SPEED ? AR_DINO_MAX_BASE_SPEED : speed;
}

static void world_update_speed_notice() {
    uint8_t next_speed_level = ar_game_score / AR_DINO_SCORE_STEP;

    if (next_speed_level > speed_level) {
        speed_level = next_speed_level;
        speed_up_notice_timer = 90;
    }
}

void ar_game_world_reset() {
    ar_game_score = 0;
    ar_game_frame_skip = 0;
    ar_game_attack_timer = 0;
    ar_game_current_speed = AR_DINO_BASE_SPEED_SCALED;
    speed_level = 0;
    speed_up_notice_timer = 0;
}

void ar_game_world_update() {
    world_update_speed_notice();

    ar_game_current_speed = world_base_speed();
    if (ar_game_attack_timer > 0) {
        ar_game_attack_timer--;
        ar_game_current_speed += AR_DINO_ATTACK_BONUS_SPEED;
    }

    if (speed_up_notice_timer > 0) {
        speed_up_notice_timer--;
    }
}

void ar_game_world_render_hud() {
    view_render.drawFastHLine(0, AR_DINO_GROUND_Y, AR_DINO_SCREEN_W, WHITE);
    view_render.drawFastHLine(0, AR_DINO_GROUND_Y + 2, AR_DINO_SCREEN_W, WHITE);
    for (uint8_t x = 4; x < AR_DINO_SCREEN_W; x += 12) {
        view_render.drawPixel(x, AR_DINO_GROUND_Y - 3, WHITE);
    }

    view_render.drawRoundRect(78, 0, 49, 12, 2, WHITE);
    view_render.setTextSize(1);
    view_render.setCursor(82, 3);
    view_render.print("S:");
    view_render.print(ar_game_score);
}

void ar_game_world_render_attack_warning() {
    if (speed_up_notice_timer > 0 && (speed_up_notice_timer % 12 < 8)) {
        view_render.drawRoundRect(1, 1, 42, 11, 2, WHITE);
        view_render.setTextSize(1);
        view_render.setCursor(5, 3);
        view_render.print("SPD UP");
    }

    if (ar_game_attack_timer > 0 && (ar_game_attack_timer % 10 < 5)) {
        view_render.setTextSize(1);
        view_render.drawRoundRect(27, 11, 74, 13, 2, WHITE);
        view_render.drawPixel(24, 17, WHITE);
        view_render.drawPixel(104, 17, WHITE);
        view_render.setCursor(35, 14);
        view_render.print("SPEED UP!");
    }
}

void ar_game_world_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case AR_GAME_WORLD_SETUP:
    case AR_GAME_WORLD_RESET: {
        APP_DBG_SIG("AR_GAME_WORLD_RESET\n");
        ar_game_world_reset();
    }
        break;

    case AR_GAME_WORLD_UPDATE: {
        ar_game_world_update();
    }
        break;

    case AR_GAME_WORLD_ATTACK_BEGIN: {
        APP_DBG_SIG("AR_GAME_WORLD_ATTACK_BEGIN\n");
        ar_game_attack_timer = AR_DINO_ATTACK_TICKS;
        BUZZER_PlayTones(tones_startup);
    }
        break;

    case AR_GAME_WORLD_LOSE: {
        APP_DBG_SIG("AR_GAME_WORLD_LOSE\n");
        ar_game_mp_state = AR_DINO_MP_LOSE;
        BUZZER_PlayTones(tones_3beep);
        SCREEN_TRAN(scr_game_over_handle, &scr_game_over);
    }
        break;

    case AR_GAME_WORLD_WIN: {
        APP_DBG_SIG("AR_GAME_WORLD_WIN\n");
        ar_game_mp_state = AR_DINO_MP_WIN;
        BUZZER_PlayTones(tones_cc);
        SCREEN_TRAN(scr_game_over_handle, &scr_game_over);
    }
        break;

    default:
        break;
    }
}
