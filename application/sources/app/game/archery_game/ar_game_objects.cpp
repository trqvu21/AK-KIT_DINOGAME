#include "ar_game_objects.h"

#include <stdlib.h>

#include "app_dbg.h"
#include "ar_game_dino.h"
#include "buzzer.h"
#include "screens_bitmap.h"
#include "task.h"
#include "task_list.h"
#include "view_render.h"

ar_game_object_t ar_game_objects[AR_DINO_OBJECT_COUNT];

static void object_set(ar_game_object_t* obj, int32_t x, int16_t y, uint8_t w, uint8_t h, uint8_t type, bool active) {
    obj->x = x;
    obj->y = y;
    obj->w = w;
    obj->h = h;
    obj->type = type;
    obj->active = active;
}

void ar_game_objects_reset() {
    object_set(&ar_game_objects[0], (128 + 10) * 10, AR_DINO_GROUND_Y - 16, 16, 16, AR_DINO_OBJ_CACTUS, true);
    object_set(&ar_game_objects[1], ar_game_objects[0].x + 1500, AR_DINO_GROUND_Y - 16, 16, 16, AR_DINO_OBJ_CACTUS, true);
    object_set(&ar_game_objects[2], ar_game_objects[1].x + 1200, AR_DINO_GROUND_Y - 28, 8, 8, AR_DINO_OBJ_GIFT, true);
    object_set(&ar_game_objects[3], ar_game_objects[2].x + 1000, AR_DINO_GROUND_Y - 20, 16, 8, AR_DINO_OBJ_BIRD, true);
}

static void recycle_object(uint8_t index) {
    int32_t max_x = 0;
    for (uint8_t i = 0; i < AR_DINO_OBJECT_COUNT; i++) {
        if (i != index && ar_game_objects[i].active && ar_game_objects[i].x > max_x) {
            max_x = ar_game_objects[i].x;
        }
    }

    uint8_t score_level = ar_game_score / AR_DINO_SCORE_STEP;
    int gap_reduce = ((settingsetup.meteoroid_speed - 1) * 15) + (score_level * 4);
    int current_min_gap = AR_DINO_MIN_OBJ_GAP - gap_reduce;
    int current_max_gap = AR_DINO_MAX_OBJ_GAP - (gap_reduce * 3);
    if (current_min_gap < 45) {
        current_min_gap = 45;
    }
    if (current_max_gap < current_min_gap + 20) {
        current_max_gap = current_min_gap + 20;
    }

    int new_x = (max_x / 10) + current_min_gap + (rand() % (current_max_gap - current_min_gap));
    if (new_x < 128) {
        new_x = 128 + current_min_gap + (rand() % 50);
    }

    ar_game_objects[index].x = new_x * 10;

    uint8_t rand_type = rand() % 100;
    uint8_t gift_chance = 18;
    uint8_t bird_chance = 45 + (score_level * 3);
    if (bird_chance > 65) {
        bird_chance = 65;
    }

    if (rand_type < gift_chance) {
        ar_game_objects[index].type = AR_DINO_OBJ_GIFT;
        ar_game_objects[index].w = 8;
        ar_game_objects[index].h = 8;
        ar_game_objects[index].y = AR_DINO_GROUND_Y - 26 - (rand() % 10);
    }
    else if (rand_type < bird_chance) {
        ar_game_objects[index].type = AR_DINO_OBJ_BIRD;
        ar_game_objects[index].w = 16;
        ar_game_objects[index].h = 8;
        ar_game_objects[index].y = (rand() % 100 < 50) ? AR_DINO_GROUND_Y - 14 : AR_DINO_GROUND_Y - 20;
    }
    else {
        ar_game_objects[index].type = AR_DINO_OBJ_CACTUS;
        ar_game_objects[index].w = 16;
        ar_game_objects[index].h = 16;
        ar_game_objects[index].y = AR_DINO_GROUND_Y - 16;
    }
}

void ar_game_objects_update() {
    for (uint8_t i = 0; i < AR_DINO_OBJECT_COUNT; i++) {
        if (!ar_game_objects[i].active) {
            ar_game_objects[i].x -= ar_game_current_speed;
            if (ar_game_objects[i].x / 10 < -20) {
                ar_game_objects[i].active = true;
            }
            continue;
        }

        ar_game_objects[i].x -= ar_game_current_speed;
        int16_t obj_x = ar_game_objects[i].x / 10;

        if (obj_x < -ar_game_objects[i].w) {
            if (ar_game_objects[i].type == AR_DINO_OBJ_CACTUS || ar_game_objects[i].type == AR_DINO_OBJ_BIRD) {
                ar_game_score++;
            }
            recycle_object(i);
        }

        if (ar_game_dino_hit_test(&ar_game_objects[i])) {
            if (ar_game_objects[i].type == AR_DINO_OBJ_CACTUS || ar_game_objects[i].type == AR_DINO_OBJ_BIRD) {
                task_post_pure_msg(AR_GAME_RF_ID, AR_GAME_RF_SEND_DIED);
                task_post_pure_msg(AR_GAME_WORLD_ID, AR_GAME_WORLD_LOSE);
            }
            else if (ar_game_objects[i].type == AR_DINO_OBJ_GIFT) {
                ar_game_objects[i].active = false;
                ar_game_score += 5;
                task_post_pure_msg(AR_GAME_RF_ID, AR_GAME_RF_SEND_ATTACK);
                BUZZER_PlayTones(tones_cc);
            }
        }
    }
}

void ar_game_objects_render() {
    for (uint8_t i = 0; i < AR_DINO_OBJECT_COUNT; i++) {
        if (ar_game_objects[i].active) {
            const unsigned char* bmp = bitmap_cactus;
            int16_t obj_x = ar_game_objects[i].x / 10;

            if (ar_game_objects[i].type == AR_DINO_OBJ_BIRD) {
                bmp = bitmap_bird;
            }
            else if (ar_game_objects[i].type == AR_DINO_OBJ_GIFT) {
                bmp = bitmap_gift;
                if ((ar_game_frame_skip & 0x01) == 0) {
                    view_render.drawPixel(obj_x - 2, ar_game_objects[i].y + 1, WHITE);
                    view_render.drawPixel(obj_x + ar_game_objects[i].w + 1, ar_game_objects[i].y + 2, WHITE);
                    view_render.drawPixel(obj_x + 3, ar_game_objects[i].y - 2, WHITE);
                }
            }

            view_render.drawBitmap(obj_x,
                                   ar_game_objects[i].y,
                                   bmp,
                                   ar_game_objects[i].w,
                                   ar_game_objects[i].h,
                                   WHITE);
        }
    }
}

void ar_game_objects_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case AR_GAME_OBJECTS_SETUP:
    case AR_GAME_OBJECTS_RESET: {
        APP_DBG_SIG("AR_GAME_OBJECTS_RESET\n");
        ar_game_objects_reset();
    }
        break;

    case AR_GAME_OBJECTS_UPDATE: {
        ar_game_objects_update();
    }
        break;

    default:
        break;
    }
}
