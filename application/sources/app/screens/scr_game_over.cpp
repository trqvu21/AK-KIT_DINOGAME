#include "scr_game_over.h"

// Gọi biến lưu điểm hiện tại của ván game từ file logic sang
extern uint32_t ar_game_score; 

/*****************************************************************************/
/* Variable Declaration - Game Over */
/*****************************************************************************/
// Biến tạm để lấy/ghi điểm Kỷ lục từ bộ nhớ
static ar_game_score_t gamescore_over; 

/*****************************************************************************/
/* View - Game Over (Nền đen - Tối giản, không có Dino) */
/*****************************************************************************/
static void view_scr_game_over();

view_dynamic_t dyn_view_item_game_over = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_over
};

view_screen_t scr_game_over = {
	&dyn_view_item_game_over,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

void view_scr_game_over() {
	view_render.clear(); // Xóa sạch màn hình thành nền đen
	
	// 1. Chữ GAME OVER to đùng ở giữa trên cùng (Cỡ chữ 2)
	view_render.setTextSize(2);
	view_render.setTextColor(WHITE);
	view_render.setCursor(12, 5);
	view_render.print("GAME OVER");

	// (Đã xóa hình Dino dead ở đây)

	// 2. Hiển thị Điểm của ván vừa chơi (SCORE) và Điểm Kỷ lục (BEST)
	// Đã đẩy tọa độ Y lên cao một chút (28 và 38) để màn hình cân đối hơn
	view_render.setTextSize(1);
	view_render.setCursor(20, 28);
	view_render.print("SCORE:");
	view_render.setCursor(65, 28);
	view_render.print(ar_game_score);

	view_render.setCursor(20, 38);
	view_render.print("BEST :");
	view_render.setCursor(65, 38);
	view_render.print(gamescore_over.score_1st);

	// 3. Vẽ 3 icon hướng dẫn ĐÚNG THỨ TỰ PHÍM TRÊN MẠCH (Trái -> Phải)
	view_render.drawBitmap(25, 53, icon_restart, 15, 15, WHITE); // Phím DOWN (Trái)
	view_render.drawBitmap(56, 53, icon_charts, 17, 15, WHITE);  // Phím UP (Giữa)
	view_render.drawBitmap(87, 53, icon_go_home, 16, 16, WHITE); // Phím MODE (Phải)

	view_render.update();
}

/*****************************************************************************/
/* Handle - Game Over */
/*****************************************************************************/
void scr_game_over_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		APP_DBG_SIG("SCREEN_ENTRY\n");
		
		// 1. Đọc bảng điểm hiện tại từ bộ nhớ EEPROM
		eeprom_read(EEPROM_SCORE_START_ADDR, (uint8_t*)&gamescore_over, sizeof(gamescore_over));

		// 2. Thuật toán so sánh và cập nhật TOP 3 (Auto-save)
		bool is_new_record = false;
		
		if (ar_game_score > gamescore_over.score_1st) {
			gamescore_over.score_3rd = gamescore_over.score_2nd;
			gamescore_over.score_2nd = gamescore_over.score_1st;
			gamescore_over.score_1st = ar_game_score;
			is_new_record = true;
		} 
		else if (ar_game_score > gamescore_over.score_2nd && ar_game_score < gamescore_over.score_1st) {
			gamescore_over.score_3rd = gamescore_over.score_2nd;
			gamescore_over.score_2nd = ar_game_score;
			is_new_record = true;
		}
		else if (ar_game_score > gamescore_over.score_3rd && ar_game_score < gamescore_over.score_2nd) {
			gamescore_over.score_3rd = ar_game_score;
			is_new_record = true;
		}

		// 3. Nếu có kỷ lục mới thì ghi ngay xuống EEPROM
		if (is_new_record) {
			eeprom_write(EEPROM_SCORE_START_ADDR, (uint8_t*)&gamescore_over, sizeof(gamescore_over));
		}

		view_render.initialize();
		view_render_display_on();
	}
		break;

	case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
		APP_DBG_SIG("RESTART_GAME\n");
		SCREEN_TRAN(scr_archery_game_handle, &scr_archery_game);
		BUZZER_PlayTones(tones_startup);
	}
		break;

	case AC_DISPLAY_BUTTON_UP_RELEASED: {
		APP_DBG_SIG("GO_CHARTS\n");
		SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game);
		BUZZER_PlayTones(tones_cc);
	}
		break;

	case AC_DISPLAY_BUTTON_MODE_RELEASED: {
		APP_DBG_SIG("GO_HOME\n");
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
		BUZZER_PlayTones(tones_cc);
	}
		break;

	default:
		break;
	}
}