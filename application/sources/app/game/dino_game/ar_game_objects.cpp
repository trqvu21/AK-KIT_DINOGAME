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

static bool object_is_obstacle(const ar_game_object_t* obj) {
    return obj->type == AR_DINO_OBJ_CACTUS || obj->type == AR_DINO_OBJ_BIRD;
}

static int object_score_level() {
    return ar_game_score / AR_DINO_SCORE_STEP;
}

static int object_gap_reduce() {
    return ((settingsetup.meteoroid_speed - 1) * 15) + (object_score_level() * 4);
}

static int object_min_gap() {
    int gap = AR_DINO_MIN_OBJ_GAP - object_gap_reduce();
    return gap < 45 ? 45 : gap;
}

static int object_max_gap(int min_gap) {
    int gap = AR_DINO_MAX_OBJ_GAP - (object_gap_reduce() * 3);
    return gap < min_gap + 20 ? min_gap + 20 : gap;
}

static int32_t object_rightmost_x(uint8_t skip_index) {
    int32_t max_x = 0;

    for (uint8_t i = 0; i < AR_DINO_OBJECT_COUNT; i++) {
        if (i != skip_index && ar_game_objects[i].active && ar_game_objects[i].x > max_x) {
            max_x = ar_game_objects[i].x;
        }
    }

    return max_x;
}

static void recycle_object(uint8_t index) {
    int current_min_gap = object_min_gap();
    int current_max_gap = object_max_gap(current_min_gap);

    int new_x = (object_rightmost_x(index) / 10) + current_min_gap + (rand() % (current_max_gap - current_min_gap));
    if (new_x < 128) {
        new_x = 128 + current_min_gap + (rand() % 50);
    }

    ar_game_objects[index].x = new_x * 10;

    uint8_t score_level = object_score_level();
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

static void handle_object_collision(ar_game_object_t* obj) {
    if (!ar_game_dino_hit_test(obj)) {
        return;
    }

    if (object_is_obstacle(obj)) {
        task_post_pure_msg(AR_GAME_RF_ID, AR_GAME_RF_SEND_DIED);
        task_post_pure_msg(AR_GAME_WORLD_ID, AR_GAME_WORLD_LOSE);
        return;
    }

    if (obj->type == AR_DINO_OBJ_GIFT) {
        obj->active = false;
        ar_game_score += 5;
        task_post_pure_msg(AR_GAME_RF_ID, AR_GAME_RF_SEND_ATTACK);
        BUZZER_PlayTones(tones_cc);
    }
}

void ar_game_objects_reset() {
    object_set(&ar_game_objects[0], (128 + 10) * 10, AR_DINO_GROUND_Y - 16, 16, 16, AR_DINO_OBJ_CACTUS, true);
    object_set(&ar_game_objects[1], ar_game_objects[0].x + 1500, AR_DINO_GROUND_Y - 16, 16, 16, AR_DINO_OBJ_CACTUS, true);
    object_set(&ar_game_objects[2], ar_game_objects[1].x + 1200, AR_DINO_GROUND_Y - 28, 8, 8, AR_DINO_OBJ_GIFT, true);
    object_set(&ar_game_objects[3], ar_game_objects[2].x + 1000, AR_DINO_GROUND_Y - 20, 16, 8, AR_DINO_OBJ_BIRD, true);
}

void ar_game_objects_update() {
    for (uint8_t i = 0; i < AR_DINO_OBJECT_COUNT; i++) {
        ar_game_object_t* obj = &ar_game_objects[i];
        obj->x -= ar_game_current_speed;

        if (!obj->active) {
            obj->active = (obj->x / 10 < -20);
            continue;
        }

        if (obj->x / 10 < -obj->w) {
            if (object_is_obstacle(obj)) {
                ar_game_score++;
            }
            recycle_object(i);
        }

        handle_object_collision(obj);
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
