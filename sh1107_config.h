#ifndef SH1107_SH1107_CONFIG_H
#define SH1107_SH1107_CONFIG_H

#include "sh1107_commands.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SH1107_SCREEN_WIDTH 128U
#define SH1107_BYTE_HEIGHT 5U
#define SH1107_BYTE_WIDTH 7U
#define SH1107_SCREEN_HEIGHT 128U
#define SH1107_FRAME_BUF_SIZE (SH1107_SCREEN_WIDTH * (SH1107_SCREEN_HEIGHT / 8))

typedef enum {
    SH1107_ERR_OK = 0,
    SH1107_ERR_FAIL = 1 << 0,
    SH1107_ERR_NULL = 1 << 1,
} sh1107_err_t;

typedef enum {
    SH1107_CONTROL_SELECT_DISPLAY = 0b01,
    SH1107_CONTROL_SELECT_COMMAND = 0b00,
} sh1107_control_select_t;

typedef struct {
    uint32_t control_pin;
    uint32_t reset_pin;

    uint8_t (*font)[5];
    uint8_t font_chars;

    uint8_t font_width;
    uint8_t font_height;
    uint8_t line_height;
    uint8_t char_width;
} sh1107_config_t;

typedef struct {
    void* gpio_user;
    sh1107_err_t (*gpio_init)(void*);
    sh1107_err_t (*gpio_deinit)(void*);
    sh1107_err_t (*gpio_write)(void*, uint32_t, bool);

    void* bus_user;
    sh1107_err_t (*bus_init)(void*);
    sh1107_err_t (*bus_deinit)(void*);
    sh1107_err_t (*bus_transmit)(void*, uint8_t const*, size_t);
} sh1107_interface_t;

#endif // SH1107_SH1107_CONFIG_H