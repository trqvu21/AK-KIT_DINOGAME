#ifndef __AR_GAME_COMMON_H__
#define __AR_GAME_COMMON_H__

#include <stdint.h>
#include <stdbool.h>

#include "app_eeprom.h"

#define AR_DINO_SCREEN_W            (128)
#define AR_DINO_GROUND_Y            (50)

#define AR_DINO_X                   (10)
#define AR_DINO_W                   (16)
#define AR_DINO_H                   (16)
#define AR_DINO_GRAVITY_SCALED      (15)
#define AR_DINO_JUMP_SCALED         (-90)
#define AR_DINO_GROUND_Y_SCALED     (AR_DINO_GROUND_Y * 10)

#define AR_DINO_BASE_SPEED_SCALED   (35)
#define AR_DINO_SETTING_SPEED_STEP  (5)
#define AR_DINO_SPEED_STEP          (6)
#define AR_DINO_SCORE_STEP          (15)
#define AR_DINO_MAX_BASE_SPEED      (85)
#define AR_DINO_ATTACK_BONUS_SPEED  (20)
#define AR_DINO_ATTACK_TICKS        (300)
#define AR_DINO_BG_SPEED_SCALED     (10)

#define AR_DINO_MIN_OBJ_GAP         (100)
#define AR_DINO_MAX_OBJ_GAP         (250)
#define AR_DINO_OBJECT_COUNT        (4)

typedef enum {
    AR_DINO_MP_WAITING = 0,
    AR_DINO_MP_PLAYING,
    AR_DINO_MP_LOSE,
    AR_DINO_MP_WIN,
} ar_dino_mp_state_t;

typedef enum {
    AR_DINO_OBJ_CACTUS = 0,
    AR_DINO_OBJ_GIFT,
    AR_DINO_OBJ_BIRD,
} ar_dino_obj_type_t;

typedef struct {
    int16_t y;
    int16_t v_y;
    bool is_jumping;
    bool is_ducking;
} ar_game_dino_t;

typedef struct {
    int32_t x;
    int16_t y;
    uint8_t w;
    uint8_t h;
    uint8_t type;
    bool active;
} ar_game_object_t;

typedef struct {
    int32_t x;
    int16_t y;
    uint8_t w;
    uint8_t h;
} ar_game_bg_t;

extern uint8_t ar_game_state;
extern uint32_t ar_game_score;
extern ar_game_setting_t settingsetup;
extern uint8_t ar_game_mp_state;
extern uint8_t ar_game_frame_skip;
extern int32_t ar_game_current_speed;
extern uint16_t ar_game_attack_timer;

#endif
