#include "ar_game_rf.h"

#include <stdlib.h>
#include <string.h>

#include "app_dbg.h"
#include "ar_game_background.h"
#include "ar_game_dino.h"
#include "ar_game_objects.h"
#include "ar_game_world.h"
#include "buzzer.h"
#include "nRF24.h"
#include "task.h"
#include "task_list.h"
#include "view_render.h"

extern void rf_init_hardware_kit(void);
extern uint8_t current_rf_channel;

static uint8_t RF_ADDR[] = { 0xE7, 0xE7, 0xE7, 0xE7, 0xE7 };

#define CMD_START    1
#define CMD_I_DIED   2
#define CMD_ATTACK   3
#define CMD_READY    4
#define CMD_ACCEPT   5
#define CMD_REJECT   6

static char my_name[4] = "P00";
static char opponent_name[4] = "";
static uint8_t lobby_state = 0;

static void rf_mode_rx() {
    nRF24_RXMode(nRF24_RX_PIPE0,
                 nRF24_ENAA_P0,
                 current_rf_channel,
                 nRF24_DataRate_1Mbps,
                 nRF24_CRC_2byte,
                 RF_ADDR,
                 5,
                 5,
                 nRF24_TXPower_0dBm);
}

static void rf_send_cmd(uint8_t cmd) {
    static uint8_t tx_buf[5];
    tx_buf[0] = cmd;
    tx_buf[1] = my_name[0];
    tx_buf[2] = my_name[1];
    tx_buf[3] = my_name[2];
    tx_buf[4] = '\0';

    nRF24_TXMode(5,
                 15,
                 current_rf_channel,
                 nRF24_DataRate_1Mbps,
                 nRF24_TXPower_0dBm,
                 nRF24_CRC_2byte,
                 nRF24_PWR_Up,
                 RF_ADDR,
                 5);
    nRF24_TXPacket(tx_buf, 5);
    rf_mode_rx();
}

static void init_player_name() {
    static bool name_initialized = false;
    if (!name_initialized) {
        uint32_t safe_random_seed = *(volatile uint32_t*)0xE000E018;
        srand(safe_random_seed);
        my_name[0] = 'P';
        my_name[1] = '0' + (rand() % 10);
        my_name[2] = '0' + (rand() % 10);
        my_name[3] = '\0';
        name_initialized = true;
    }
}

static void reset_game_modules() {
    ar_game_world_reset();
    ar_game_dino_reset();
    ar_game_objects_reset();
    ar_game_background_reset();
}

static void start_match() {
    reset_game_modules();
    ar_game_mp_state = AR_DINO_MP_PLAYING;
    BUZZER_PlayTones(tones_startup);
}

static bool packet_is_from_opponent(const char* rx_name) {
    return strcmp(rx_name, opponent_name) == 0 || opponent_name[0] == '\0';
}

static void handle_waiting_packet(uint8_t cmd, const char* rx_name) {
    if (cmd == CMD_READY) {
        if (lobby_state == 0) {
            strcpy(opponent_name, rx_name);
            lobby_state = 2;
            BUZZER_PlayTones(tones_cc);
        }
        else if (lobby_state == 1) {
            strcpy(opponent_name, rx_name);
            rf_send_cmd(CMD_ACCEPT);
            start_match();
        }
    }
    else if (cmd == CMD_ACCEPT && lobby_state == 1) {
        strcpy(opponent_name, rx_name);
        start_match();
    }
    else if (cmd == CMD_REJECT && lobby_state == 1) {
        lobby_state = 0;
        BUZZER_PlayTones(tones_3beep);
    }
}

static void handle_playing_packet(uint8_t cmd, const char* rx_name) {
    if (!packet_is_from_opponent(rx_name)) {
        return;
    }

    if (cmd == CMD_I_DIED) {
        task_post_pure_msg(AR_GAME_WORLD_ID, AR_GAME_WORLD_WIN);
    }
    else if (cmd == CMD_ATTACK) {
        task_post_pure_msg(AR_GAME_WORLD_ID, AR_GAME_WORLD_ATTACK_BEGIN);
    }
}

void ar_game_rf_poll() {
    static uint8_t rx_data[5];

    if (nRF24_RXPacket(rx_data, 5) == nRF24_RX_PCKT_PIPE0) {
        uint8_t cmd = rx_data[0];
        char rx_name[4] = { (char)rx_data[1], (char)rx_data[2], (char)rx_data[3], '\0' };

        if (ar_game_mp_state == AR_DINO_MP_WAITING) {
            handle_waiting_packet(cmd, rx_name);
        }
        else if (ar_game_mp_state == AR_DINO_MP_PLAYING) {
            handle_playing_packet(cmd, rx_name);
        }
    }
}

void ar_game_rf_setup() {
    init_player_name();
    lobby_state = 0;
    opponent_name[0] = '\0';
    rf_init_hardware_kit();
    rf_mode_rx();
}

void ar_game_rf_ready() {
    if (ar_game_mp_state == AR_DINO_MP_WAITING) {
        if (lobby_state == 0) {
            lobby_state = 1;
            rf_send_cmd(CMD_READY);
            BUZZER_PlayTones(tones_cc);
        }
        else if (lobby_state == 1) {
            lobby_state = 3;
            opponent_name[0] = '\0';
            rf_send_cmd(CMD_START);
            start_match();
        }
        else if (lobby_state == 2) {
            ar_game_rf_reject();
        }
    }
}

void ar_game_rf_accept() {
    if (ar_game_mp_state == AR_DINO_MP_WAITING && lobby_state == 2) {
        rf_send_cmd(CMD_ACCEPT);
        start_match();
    }
}

void ar_game_rf_reject() {
    if (ar_game_mp_state == AR_DINO_MP_WAITING && lobby_state == 2) {
        rf_send_cmd(CMD_REJECT);
        lobby_state = 0;
        BUZZER_PlayTones(tones_cc);
    }
}

void ar_game_rf_start_solo() {
    if (ar_game_mp_state == AR_DINO_MP_WAITING && lobby_state == 1) {
        lobby_state = 3;
        opponent_name[0] = '\0';
        rf_send_cmd(CMD_START);
        start_match();
    }
}

void ar_game_rf_render_lobby() {
    view_render.setTextSize(1);
    if (lobby_state == 0) {
        view_render.setCursor(15, 15);
        view_render.print("MY NAME: [");
        view_render.print(my_name);
        view_render.print("]");
        view_render.setCursor(15, 35);
        view_render.print("BTN DOWN TO READY");
    }
    else if (lobby_state == 1) {
        view_render.setCursor(15, 15);
        view_render.print("WAITING REPLY...");
        view_render.setCursor(15, 35);
        view_render.print("BTN DOWN TO SOLO");
    }
    else if (lobby_state == 2) {
        view_render.setCursor(10, 10);
        view_render.print("[");
        view_render.print(opponent_name);
        view_render.print("] INVITES!");
        view_render.setCursor(10, 30);
        view_render.print("UP  : ACCEPT");
        view_render.setCursor(10, 45);
        view_render.print("DOWN: REJECT");
    }
}

void ar_game_rf_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case AR_GAME_RF_SETUP: {
        APP_DBG_SIG("AR_GAME_RF_SETUP\n");
        ar_game_rf_setup();
    }
        break;

    case AR_GAME_RF_POLL: {
        ar_game_rf_poll();
    }
        break;

    case AR_GAME_RF_READY: {
        APP_DBG_SIG("AR_GAME_RF_READY\n");
        ar_game_rf_ready();
    }
        break;

    case AR_GAME_RF_ACCEPT: {
        APP_DBG_SIG("AR_GAME_RF_ACCEPT\n");
        ar_game_rf_accept();
    }
        break;

    case AR_GAME_RF_REJECT: {
        APP_DBG_SIG("AR_GAME_RF_REJECT\n");
        ar_game_rf_reject();
    }
        break;

    case AR_GAME_RF_START_SOLO: {
        APP_DBG_SIG("AR_GAME_RF_START_SOLO\n");
        ar_game_rf_start_solo();
    }
        break;

    case AR_GAME_RF_SEND_ATTACK: {
        APP_DBG_SIG("AR_GAME_RF_SEND_ATTACK\n");
        rf_send_cmd(CMD_ATTACK);
    }
        break;

    case AR_GAME_RF_SEND_DIED: {
        APP_DBG_SIG("AR_GAME_RF_SEND_DIED\n");
        rf_send_cmd(CMD_I_DIED);
    }
        break;

    case AR_GAME_RF_RESET: {
        APP_DBG_SIG("AR_GAME_RF_RESET\n");
        lobby_state = 0;
        opponent_name[0] = '\0';
        rf_mode_rx();
    }
        break;

    default:
        break;
    }
}
