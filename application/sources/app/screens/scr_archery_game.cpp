#include "scr_archery_game.h"
#include "screens.h"

/*****************************************************************************/
/* Variable Declaration - Dino Run Game */
/*****************************************************************************/
// Cấu hình vật lý & Game
#define GROUND_Y        50
#define DINO_X          10
#define DINO_W          16
#define DINO_H          16
#define CACTUS_W        8
#define CACTUS_H        16

#define GRAVITY         2
#define JUMP_FORCE      -12 

// Biến toàn cục
uint8_t ar_game_state; 
ar_game_setting_t settingsetup; // Giữ để tương thích hệ thống cũ

// Struct đối tượng
typedef struct {
    int16_t y;
    int16_t v_y;
    bool is_jumping;
} dino_t;

typedef struct {
    int16_t x;
    bool active;
} cactus_t;

static dino_t dino;
static cactus_t cactus[2];

/*****************************************************************************/
/* Logic Functions */
/*****************************************************************************/
void dino_reset() {
    dino.y = GROUND_Y - DINO_H;
    dino.v_y = 0;
    dino.is_jumping = false;

    // Đặt vị trí ban đầu xa một chút để người chơi kịp chuẩn bị
    cactus[0].x = 128 + 50;
    cactus[0].active = true;
    
    // Cây thứ 2 cách cây 1 ít nhất 100px
    cactus[1].x = cactus[0].x + 100 + (rand() % 50); 
    cactus[1].active = true;

    ar_game_score = 0;
}

void dino_update() {
    // 1. Vật lý Dino (Nhảy & Rơi)
    if (dino.is_jumping) {
        dino.y += dino.v_y;
        dino.v_y += GRAVITY;

        if (dino.y >= (GROUND_Y - DINO_H)) {
            dino.y = GROUND_Y - DINO_H;
            dino.is_jumping = false;
            dino.v_y = 0;
        }
    }

    // 2. Di chuyển cây xương rồng
    for (int i = 0; i < 2; i++) {
        if (cactus[i].active) {
            cactus[i].x -= 6; // Tốc độ di chuyển nền

            // KHI CÂY CHẠY HẾT MÀN HÌNH -> TÁI TẠO Ở BÊN PHẢI
            if (cactus[i].x < -CACTUS_W) {
                // Random vị trí mới: 
                // 128 (mép phải) + 90 (khoảng cách an toàn) + Random(0-60)
                cactus[i].x = 128 + 90 + (rand() % 60);
                
                ar_game_score++;
                BUZZER_PlayTones(tones_cc); // Âm thanh ghi điểm
            }

            // 3. Xử lý va chạm (Collision Detection)
            if ((DINO_X + DINO_W - 4 > cactus[i].x) && 
                (DINO_X + 2 < cactus[i].x + CACTUS_W) &&
                (dino.y + DINO_H > GROUND_Y - CACTUS_H + 4)) {
                
                // Gửi tín hiệu Reset (Game Over)
                task_post_pure_msg(AC_TASK_DISPLAY_ID, AR_GAME_RESET);
            }
        }
    }
}

/*****************************************************************************/
/* View Functions */
/*****************************************************************************/
static void view_scr_dino_game();

view_dynamic_t dyn_view_item_archery_game = {
    { .item_type = ITEM_TYPE_DYNAMIC },
    view_scr_dino_game
};

view_screen_t scr_archery_game = {
    &dyn_view_item_archery_game,
    ITEM_NULL, ITEM_NULL,
    .focus_item = 0,
};

void view_scr_dino_game() {
    if (ar_game_state == GAME_PLAY) {
        // Vẽ mặt đất
        view_render.drawFastHLine(0, GROUND_Y, 128, WHITE);
        
        // Vẽ điểm số
        view_render.setTextSize(1);
        view_render.setTextColor(WHITE);
        view_render.setCursor(90, 2);
        view_render.print(ar_game_score);

        // Vẽ Dino (Bitmap)
        view_render.drawBitmap(DINO_X, dino.y, bitmap_dino, DINO_W, DINO_H, WHITE);

        // Vẽ Xương rồng (Bitmap)
        for (int i = 0; i < 2; i++) {
            view_render.drawBitmap(cactus[i].x, GROUND_Y - CACTUS_H, bitmap_cactus, CACTUS_W, CACTUS_H, WHITE);
        }
    }
    else if (ar_game_state == GAME_OVER) {
        view_render.clear();
        view_render.setTextSize(2);
        view_render.setTextColor(WHITE);
        view_render.setCursor(17, 24);
        view_render.print("YOU LOSE");
        
        view_render.setTextSize(1);
        view_render.setCursor(35, 45);
        view_render.print("Score: ");
        view_render.print(ar_game_score);
    }
}

/*****************************************************************************/
/* Handle Functions */
/*****************************************************************************/
void scr_archery_game_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case SCREEN_ENTRY: {
        APP_DBG_SIG("DINO: START\n");
        dino_reset();
        // Timer chạy game: 50ms (20 FPS)
        timer_set(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK, 50, TIMER_PERIODIC);
        ar_game_state = GAME_PLAY;
    }
        break;

    case AR_GAME_TIME_TICK: {
        if (ar_game_state == GAME_PLAY) {
            dino_update();
        }
    }
        break;

    // --- XỬ LÝ NÚT BẤM (Dùng sự kiện PRESSED để nhảy ngay lập tức) ---
    // Yêu cầu: Bạn đã bỏ comment phần gửi tín hiệu PRESSED trong app_bsp.cpp
    case AC_DISPLAY_BUTTON_UP_PRESSED: 
    case AC_DISPLAY_BUTTON_DOWN_PRESSED: {
        if (ar_game_state == GAME_PLAY) {
            if (!dino.is_jumping) {
                dino.v_y = JUMP_FORCE; // Nhảy
                dino.is_jumping = true;
                BUZZER_PlayTones(tones_cc);
            }
        }
    }
        break;

    // Dùng sự kiện RELEASED cho việc chơi lại (để tránh bị double click)
    case AC_DISPLAY_BUTTON_UP_RELEASED:
    case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
        if (ar_game_state == GAME_OVER) {
             dino_reset();
             ar_game_state = GAME_PLAY;
        }
    }
        break;

    case AC_DISPLAY_BUTTON_MODE_RELEASED: {
        // Thoát game về Menu
        timer_remove_attr(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK);
        SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
    }
        break;

    case AR_GAME_RESET: {
        // Xử lý khi thua
        ar_game_state = GAME_OVER;
        BUZZER_PlayTones(tones_3beep);
        eeprom_write(EEPROM_SCORE_PLAY_ADDR, (uint8_t*)&ar_game_score, sizeof(ar_game_score));
    }
        break;

    default:
        break;
    }
}