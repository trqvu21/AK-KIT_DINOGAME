#include "scr_game_over.h"

#include "ar_game_common.h"

extern uint32_t ar_game_score;

static ar_game_score_t gamescore_over;
static bool is_new_record = false;

static int8_t anim_y = 0;
static int8_t anim_dir = 1;
static bool blink_state = true;

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
	view_render.clear();

	view_render.setTextColor(WHITE);
	view_render.drawFastHLine(8, 3, 112, WHITE);
	view_render.drawPixel(4, 3, WHITE);
	view_render.drawPixel(123, 3, WHITE);

	if (ar_game_mp_state == AR_DINO_MP_WIN) {
		int x = 8, y = 8 + anim_y;
		view_render.drawRoundRect(x + 3, y, 10, 8, 2, WHITE);
		view_render.drawFastHLine(x + 5, y + 8, 6, WHITE);
		view_render.drawFastVLine(x + 8, y + 8, 4, WHITE);
		view_render.drawFastHLine(x + 4, y + 12, 10, WHITE);
		view_render.drawFastVLine(x, y + 2, 4, WHITE);
		view_render.drawFastVLine(x + 16, y + 2, 4, WHITE);
		view_render.drawLine(x, y + 2, x + 3, y + 4, WHITE);
		view_render.drawLine(x + 16, y + 2, x + 12, y + 4, WHITE);
		view_render.setTextSize(2);
		view_render.setCursor(34, 9 + anim_y);
		view_render.print("YOU WIN");
	}
	else {
		int x = 8, y = 8 + anim_y;
		view_render.drawRoundRect(x + 2, y, 14, 12, 3, WHITE);
		view_render.drawPixel(x + 6, y + 5, WHITE);
		view_render.drawPixel(x + 12, y + 5, WHITE);
		view_render.drawFastHLine(x + 7, y + 9, 5, WHITE);
		view_render.drawFastVLine(x + 5, y + 12, 4, WHITE);
		view_render.drawFastVLine(x + 9, y + 12, 4, WHITE);
		view_render.drawFastVLine(x + 13, y + 12, 4, WHITE);
		view_render.setTextSize(2);
		view_render.setCursor(28, 9 + anim_y);
		view_render.print("YOU LOSE");
	}

	view_render.setTextSize(1);
	view_render.drawRoundRect(16, 27, 96, 19, 2, WHITE);
	view_render.drawFastVLine(62, 29, 15, WHITE);
	view_render.setCursor(22, 32);
	view_render.print("SCORE");
	view_render.setCursor(72, 32);
	view_render.print(ar_game_score);

	if (is_new_record) {

		if (blink_state) {
			view_render.fillRect(33, 46, 62, 8, WHITE);
			view_render.setTextColor(BLACK);
			view_render.setCursor(38, 47);
			view_render.print("NEW BEST");
			view_render.setTextColor(WHITE);
			view_render.setCursor(99, 47);
			view_render.print(gamescore_over.score_1st);
		}
	}
	else {

		view_render.setCursor(32, 47);
		view_render.print("BEST :");
		view_render.setCursor(75, 47);
		view_render.print(gamescore_over.score_1st);
	}

	int x1 = 12, y1 = 54;
	view_render.drawFastHLine(x1+2, y1+1, 4, WHITE);
	view_render.drawFastHLine(x1+2, y1+7, 4, WHITE);
	view_render.drawFastVLine(x1+1, y1+3, 3, WHITE);
	view_render.drawFastVLine(x1+7, y1+4, 2, WHITE);
	view_render.drawPixel(x1+2, y1+2, WHITE);
	view_render.drawPixel(x1+2, y1+6, WHITE);
	view_render.drawPixel(x1+6, y1+6, WHITE);
	view_render.drawLine(x1+5, y1-1, x1+7, y1+1, WHITE);
	view_render.drawLine(x1+5, y1+3, x1+7, y1+1, WHITE);

	int x2 = 58, y2 = 54;

	view_render.drawFastHLine(x2, y2+4, 3, WHITE);
	view_render.drawFastVLine(x2, y2+4, 5, WHITE);

	view_render.drawFastHLine(x2+3, y2, 4, WHITE);
	view_render.drawFastVLine(x2+3, y2, 9, WHITE);
	view_render.drawFastVLine(x2+7, y2, 9, WHITE);

	view_render.drawFastHLine(x2+8, y2+6, 3, WHITE);
	view_render.drawFastVLine(x2+11, y2+6, 3, WHITE);
	view_render.drawFastHLine(x2, y2+9, 12, WHITE);

	int x3 = 106, y3 = 54;
	view_render.drawLine(x3, y3+4, x3+4, y3, WHITE);
	view_render.drawLine(x3+4, y3, x3+8, y3+4, WHITE);
	view_render.drawFastVLine(x3+1, y3+4, 5, WHITE);
	view_render.drawFastVLine(x3+7, y3+4, 5, WHITE);
	view_render.drawFastHLine(x3+1, y3+8, 7, WHITE);
	view_render.drawFastVLine(x3+3, y3+6, 3, WHITE);
	view_render.drawFastVLine(x3+5, y3+6, 3, WHITE);

	view_render.update();
}

void scr_game_over_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		APP_DBG_SIG("SCREEN_ENTRY\n");

		eeprom_read(EEPROM_SCORE_START_ADDR, (uint8_t*)&gamescore_over, sizeof(gamescore_over));
		is_new_record = false;

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

		if (is_new_record) {
			eeprom_write(EEPROM_SCORE_START_ADDR, (uint8_t*)&gamescore_over, sizeof(gamescore_over));
		}

		anim_y = 0;
		anim_dir = 1;
		blink_state = true;

		timer_set(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK, 100, TIMER_ONE_SHOT);

		view_render.initialize();
		view_render_display_on();
	}
		break;

	case AR_GAME_TIME_TICK: {

		anim_y += anim_dir;
		if (anim_y >= 2 || anim_y <= -2) {
			anim_dir = -anim_dir;
		}

		blink_state = !blink_state;

		view_scr_game_over();

		timer_set(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK, 150, TIMER_ONE_SHOT);
		break;
	}

	case AC_DISPLAY_BUTTON_DOWN_RELEASED: {
		APP_DBG_SIG("RESTART_GAME\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK);
		SCREEN_TRAN(scr_dino_game_handle, &scr_dino_game);
		BUZZER_PlayTones(tones_startup);
	}
		break;

	case AC_DISPLAY_BUTTON_UP_RELEASED: {
		APP_DBG_SIG("GO_CHARTS\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK);
		SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game);
		BUZZER_PlayTones(tones_cc);
	}
		break;

	case AC_DISPLAY_BUTTON_MODE_RELEASED: {
		APP_DBG_SIG("GO_HOME\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AR_GAME_TIME_TICK);
		SCREEN_TRAN(scr_menu_game_handle, &scr_menu_game);
		BUZZER_PlayTones(tones_cc);
	}
		break;

	default:
		break;
	}
}
