#include "sh1107.h"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// sh1107_err_t sh1107_initialize()
// {
//     sh1107_device_reset();

//     sh1107_bus_transmit_command(sh1107,(0xAE); // Display OFF
//     sh1107_bus_transmit_command(sh1107,(0xD5); // Set Display Clock Divide
//     Ratio sh1107_bus_transmit_command(sh1107,(0x80);
//     sh1107_bus_transmit_command(sh1107,(0xA8); // Set Multiplex Ratio
//     sh1107_bus_transmit_command(sh1107,(0x7F);
//     sh1107_bus_transmit_command(sh1107,(0xD3); // Display Offset
//     sh1107_bus_transmit_command(sh1107,(0x00);
//     sh1107_bus_transmit_command(sh1107,(0x40); // Display Start Line
//     sh1107_bus_transmit_command(sh1107,(0x8D); // Charge Pump
//     sh1107_bus_transmit_command(sh1107,(0x14);
//     sh1107_bus_transmit_command(sh1107,(0xAF); // Display ON

//     sh1107_is_initialized = true;
//     sh1107_delay(100UL);
// }

static sh1107_err_t sh1107_gpio_initialize(sh1107_t const* sh1107)
{
    return sh1107->interface.gpio_initialize
               ? sh1107->interface.gpio_initialize(sh1107->interface.gpio_user)
               : SH1107_ERR_NULL;
}

static sh1107_err_t sh1107_gpio_deinitialize(sh1107_t const* sh1107)
{
    return sh1107->interface.gpio_deinitialize
               ? sh1107->interface.gpio_deinitialize(sh1107->interface.gpio_user)
               : SH1107_ERR_NULL;
}

static sh1107_err_t sh1107_gpio_write(sh1107_t const* sh1107, uint32_t pin, bool state)
{
    return sh1107->interface.gpio_write
               ? sh1107->interface.gpio_write(sh1107->interface.gpio_user, pin, state)
               : SH1107_ERR_NULL;
}

static sh1107_err_t sh1107_bus_initialize(sh1107_t const* sh1107)
{
    return sh1107->interface.bus_initialize
               ? sh1107->interface.bus_initialize(sh1107->interface.bus_user)
               : SH1107_ERR_NULL;
}

static sh1107_err_t sh1107_bus_deinitialize(sh1107_t const* sh1107)
{
    return sh1107->interface.bus_deinitialize
               ? sh1107->interface.bus_deinitialize(sh1107->interface.bus_user)
               : SH1107_ERR_NULL;
}

static sh1107_err_t sh1107_bus_transmit(sh1107_t const* sh1107,
                                        uint8_t const* data,
                                        size_t data_size)
{
    return sh1107->interface.bus_transmit
               ? sh1107->interface.bus_transmit(sh1107->interface.bus_user, data, data_size)
               : SH1107_ERR_NULL;
}

static sh1107_err_t sh1107_bus_transmit_command(sh1107_t const* sh1107,
                                                uint8_t* data,
                                                size_t data_size)
{
    sh1107_err_t err =
        sh1107_gpio_write(sh1107, sh1107->config.control_pin, SH1107_CONTROL_SELECT_COMMAND);
    err |= sh1107_bus_transmit(sh1107, data, data_size);

    return err;
}

static sh1107_err_t sh1107_bus_transmit_display(sh1107_t const* sh1107,
                                                uint8_t const* data,
                                                size_t data_size)
{
    sh1107_err_t err =
        sh1107_gpio_write(sh1107, sh1107->config.control_pin, SH1107_CONTROL_SELECT_DISPLAY);
    err |= sh1107_bus_transmit(sh1107, data, data_size);

    return err;
}

sh1107_err_t sh1107_initialize(sh1107_t* sh1107,
                               sh1107_config_t const* config,
                               sh1107_interface_t const* interface)
{
    assert(sh1107 && config && interface && config->font);

    memset(sh1107, 0, sizeof(*sh1107));
    memcpy(&sh1107->config, config, sizeof(*config));
    memcpy(&sh1107->interface, interface, sizeof(*interface));

    sh1107_err_t err = sh1107_bus_initialize(sh1107);
    err |= sh1107_gpio_initialize(sh1107);

    return err;
}

sh1107_err_t sh1107_deinitialize(sh1107_t* sh1107)
{
    assert(sh1107);

    sh1107_err_t err = sh1107_bus_deinitialize(sh1107);
    err |= sh1107_gpio_deinitialize(sh1107);

    memset(sh1107, 0, sizeof(*sh1107));

    return err;
}

sh1107_err_t sh1107_display_frame_buf(sh1107_t const* sh1107)
{
    assert(sh1107);

    sh1107_err_t err = SH1107_ERR_OK;
    uint8_t cmd = {};

    for (uint8_t page = 0; page < (SH1107_SCREEN_HEIGHT / 8); page++) {
        cmd = 0xB0 | page;
        err |= sh1107_bus_transmit_command(sh1107, &cmd, sizeof(cmd));
        cmd = 0x00;
        err |= sh1107_bus_transmit_command(sh1107, &cmd, sizeof(cmd));
        cmd = 0x10;
        err |= sh1107_bus_transmit_command(sh1107, &cmd, sizeof(cmd));

        err |= sh1107_bus_transmit_display(sh1107,
                                           sh1107->frame_buf + page * SH1107_SCREEN_WIDTH,
                                           SH1107_SCREEN_WIDTH);
    }

    return err;
}

void sh1107_clear_frame_buf(sh1107_t* sh1107)
{
    assert(sh1107);

    memset(sh1107->frame_buf, 0, sizeof(sh1107->frame_buf));
}

sh1107_err_t sh1107_set_pixel(sh1107_t* sh1107, uint8_t x, uint8_t y, bool color)
{
    assert(sh1107);

    if (x >= SH1107_SCREEN_WIDTH || y >= SH1107_SCREEN_HEIGHT) {
        return SH1107_ERR_FAIL;
    }

    size_t byte_index = (y / 8) * SH1107_SCREEN_WIDTH + x;
    uint8_t bit_mask = 1 << (y % 8);

    sh1107->frame_buf[byte_index] = color ? (sh1107->frame_buf[byte_index] | bit_mask)
                                          : (sh1107->frame_buf[byte_index] & ~bit_mask);

    return SH1107_ERR_OK;
}

sh1107_err_t sh1107_draw_line(sh1107_t* sh1107,
                              uint8_t x0,
                              uint8_t y0,
                              uint8_t x1,
                              uint8_t y1,
                              bool color)
{
    assert(sh1107);

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    int e2;

    sh1107_err_t err = SH1107_ERR_OK;

    while (1) {
        err |= sh1107_set_pixel(sh1107, x0, y0, color);
        if (x0 == x1 && y0 == y1) {
            break;
        }

        e2 = 2 * error;
        if (e2 >= dy) {
            error += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            error += dx;
            y0 += sy;
        }
    }

    return err;
}

sh1107_err_t sh1107_draw_rect(sh1107_t* sh1107,
                              uint8_t x,
                              uint8_t y,
                              uint8_t w,
                              uint8_t h,
                              bool color)
{
    assert(sh1107);

    if (h <= 0 || w <= 0) {
        return SH1107_ERR_FAIL;
    }

    int x_start = x;
    int x_end = x + w;
    int y_start = y;
    int y_end = y + h;

    sh1107_err_t err = SH1107_ERR_OK;

    for (int x = x_start; x < x_end; x++) {
        for (int y = y_start; y < y_end; y++) {
            if (color || (x <= x_start || x >= x_end - 1) || (y <= y_start || y >= y_end - 1)) {
                err |= sh1107_set_pixel(sh1107, x, y, true);
            }
        }
    }

    return err;
}

sh1107_err_t sh1107_draw_circle(sh1107_t* sh1107, uint8_t x0, uint8_t y0, uint8_t r, bool color)
{
    assert(sh1107);

    sh1107_err_t err = SH1107_ERR_OK;

    int x = r, y = 0, error = 1 - x;
    while (x >= y) {
        err |= sh1107_set_pixel(sh1107, x0 + x, y0 + y, color);
        err |= sh1107_set_pixel(sh1107, x0 + y, y0 + x, color);
        err |= sh1107_set_pixel(sh1107, x0 - y, y0 + x, color);
        err |= sh1107_set_pixel(sh1107, x0 - x, y0 + y, color);
        err |= sh1107_set_pixel(sh1107, x0 - x, y0 - y, color);
        err |= sh1107_set_pixel(sh1107, x0 - y, y0 - x, color);
        err |= sh1107_set_pixel(sh1107, x0 + y, y0 - x, color);
        err |= sh1107_set_pixel(sh1107, x0 + x, y0 - y, color);
        y++;
        if (error < 0) {
            error += 2 * y + 1;
        } else {
            x--;
            error += 2 * (y - x) + 1;
        }
    }

    return err;
}

sh1107_err_t sh1107_draw_bitmap(sh1107_t* sh1107,
                                uint8_t x,
                                uint8_t y,
                                uint8_t w,
                                uint8_t h,
                                uint8_t* bitmap,
                                size_t bitmap_size,
                                bool color)
{
    assert(sh1107);

    sh1107_err_t err = SH1107_ERR_OK;

    for (uint8_t j = 0; j < h; j++) {
        for (uint8_t i = 0; i < w; i++) {
            size_t index = j * ((w + 7) / 8) + (i / 8);
            if (index > bitmap_size) {
                break;
            }
            uint8_t byte = bitmap[index];
            if (byte & (1 << (7 - (i % 8)))) {
                err |= sh1107_set_pixel(sh1107, x + i, y + j, color);
            }
        }
    }

    return err;
}

sh1107_err_t sh1107_draw_char(sh1107_t* sh1107, uint8_t x, uint8_t y, char c)
{
    assert(sh1107);

    if (c < 32 || c > 127) {
        return SH1107_ERR_FAIL;
    }

    sh1107_err_t err = SH1107_ERR_OK;

    for (uint8_t i = 0; i < sh1107->config.font_width; i++) {
        uint8_t line = sh1107->config.font[c - 32][i];
        for (uint8_t j = 0; j < sh1107->config.font_height; j++) {
            err |= sh1107_set_pixel(sh1107, x + i, y + j, line & (1 << j));
        }
    }

    return err;
}

sh1107_err_t sh1107_draw_string(sh1107_t* sh1107, uint8_t x, uint8_t y, char const* s)
{
    assert(sh1107);

    sh1107_err_t err = SH1107_ERR_OK;

    while (*s != '\0') {
        err |= sh1107_draw_char(sh1107, x, y, *s++);
        x += sh1107->config.font_width + 1;

        if (x >= SH1107_SCREEN_WIDTH) {
            break;
        }
    }

    return err;
}

sh1107_err_t sh1107_draw_string_formatted(sh1107_t* sh1107,
                                          uint8_t x,
                                          uint8_t y,
                                          char const* fmt,
                                          ...)
{
    assert(sh1107);

    va_list args;
    va_start(args, fmt);
    size_t size = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    char* buffer = calloc(size, sizeof(char));
    if (!buffer)
        return SH1107_ERR_FAIL;

    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);

    sh1107_err_t err = sh1107_draw_string(sh1107, x, y, buffer);
    free(buffer);

    return err;
}

sh1107_err_t sh1107_device_reset(sh1107_t const* sh1107)
{
    assert(sh1107);

    sh1107_err_t err = sh1107_gpio_write(sh1107, sh1107->config.reset_pin, 0U);
    //  sh1107_delay(100UL);
    err |= sh1107_gpio_write(sh1107, sh1107->config.reset_pin, 1U);
    //  sh1107_delay(100UL);

    return err;
}

sh1107_err_t sh1107_send_set_lower_column_address_cmd(sh1107_t const* sh1107, uint8_t address)
{
    assert(sh1107);

    uint8_t data = {};

    data |= SH1107_CMD_SET_LOWER_COLUMN_ADDRESS << 4U;
    data |= address & 0x0FU;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_set_higher_column_address_cmd(sh1107_t const* sh1107, uint8_t address)
{
    assert(sh1107);

    uint8_t data = {};

    data |= SH1107_CMD_SET_HIGHER_COLUMN_ADDRESS << 3U;
    data |= address & 0x07U;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_set_memory_addressing_mode_cmd(sh1107_t const* sh1107, uint8_t mode)
{
    assert(sh1107);

    uint8_t data = {};

    data |= SH1107_CMD_SET_MEMORY_ADDRESSING_MODE << 1U;
    data |= mode & 0x01U;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_set_segment_remap_cmd(sh1107_t const* sh1107, uint8_t remap)
{
    assert(sh1107);

    uint8_t data = {};

    data |= SH1107_CMD_SET_SEGMENT_REMAP << 1U;
    data |= remap & 0x01U;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_set_entire_display_on_off_cmd(sh1107_t const* sh1107, uint8_t on_off)
{
    assert(sh1107);

    uint8_t data = {};

    data |= SH1107_CMD_SET_ENTIRE_DISPLAY_ON_OFF << 1U;
    data |= on_off & 0x01U;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_set_normal_reverse_display_cmd(sh1107_t const* sh1107, uint8_t display)
{
    assert(sh1107);

    uint8_t data = {};

    data |= SH1107_CMD_SET_NORMAL_REVERSE_DISPLAY << 1U;
    data |= display & 0x01U;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_set_display_on_off_cmd(sh1107_t const* sh1107, uint8_t on_off)
{
    assert(sh1107);

    uint8_t data = {};

    data |= SH1107_CMD_SET_DISPLAY_ON_OFF << 1U;
    data |= on_off & 0x01U;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_set_page_address_cmd(sh1107_t const* sh1107, uint8_t address)
{
    assert(sh1107);

    uint8_t data = {};

    data |= SH1107_CMD_SET_PAGE_ADDRESS << 4U;
    data |= address & 0x0FU;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_set_output_scan_direction_cmd(sh1107_t const* sh1107, uint8_t direction)
{
    assert(sh1107);

    uint8_t data = {};

    data |= SH1107_CMD_SET_OUTPUT_SCAN_DIRECTION << 4U;
    data |= (direction & 0x01U) << 3U;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_read_modify_write_cmd(sh1107_t const* sh1107)
{
    assert(sh1107);

    uint8_t data = {};

    data = SH1107_CMD_READ_MODIFY_WRITE;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_end_cmd(sh1107_t const* sh1107)
{
    assert(sh1107);

    uint8_t data = {};

    data = SH1107_CMD_END;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_nop_cmd(sh1107_t const* sh1107)
{
    assert(sh1107);

    uint8_t data = {};

    data = SH1107_CMD_NOP;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_read_id_cmd(sh1107_t const* sh1107,
                                     uint8_t busy,
                                     uint8_t on_off,
                                     uint8_t id)
{
    assert(sh1107);

    uint8_t data = {};

    data |= (busy & 0x01U) << 7U;
    data |= (on_off & 0x01U) << 6U;
    data |= id & 0x3FU;

    return sh1107_bus_transmit_command(sh1107, &data, sizeof(data));
}

sh1107_err_t sh1107_send_set_contrast_control_cmd(sh1107_t const* sh1107, uint8_t contrast)
{
    assert(sh1107);

    uint8_t data[2] = {};

    data[0] = SH1107_CMD_SET_CONTRAST_CONTROL;
    data[1] = contrast & 0xFFU;

    return sh1107_bus_transmit_command(sh1107, data, sizeof(data));
}

sh1107_err_t sh1107_send_set_multiplex_ratio_cmd(sh1107_t const* sh1107, uint8_t ratio)
{
    assert(sh1107);

    uint8_t data[2] = {};

    data[0] = SH1107_CMD_SET_MULTIPLEX_RATIO;
    data[1] = ratio & 0x3FU;

    return sh1107_bus_transmit_command(sh1107, data, sizeof(data));
}

sh1107_err_t sh1107_send_set_display_offset_cmd(sh1107_t const* sh1107, uint8_t offset)
{
    assert(sh1107);

    uint8_t data[2] = {};

    data[0] = SH1107_CMD_SET_DISPLAY_OFFSET;
    data[1] = offset & 0x3FU;

    return sh1107_bus_transmit_command(sh1107, data, sizeof(data));
}

sh1107_err_t sh1107_send_set_dc_dc_setting_cmd(sh1107_t const* sh1107, uint8_t setting)
{
    assert(sh1107);

    uint8_t data[2] = {};

    data[0] = SH1107_CMD_SET_DC_DC_SETTING >> 4U;
    data[1] = (SH1107_CMD_SET_DC_DC_SETTING & 0x0FU) | (setting & 0x0FU);

    return sh1107_bus_transmit_command(sh1107, data, sizeof(data));
}

sh1107_err_t sh1107_send_set_display_clock_cmd(sh1107_t const* sh1107,
                                               uint8_t osc_freq,
                                               uint8_t clock_divide)
{
    assert(sh1107);

    uint8_t data[2] = {};

    data[0] = SH1107_CMD_SET_DISPLAY_CLOCK;
    data[1] = ((osc_freq & 0x0FU) << 4U) | (clock_divide & 0x0FU);

    return sh1107_bus_transmit_command(sh1107, data, sizeof(data));
}

sh1107_err_t sh1107_send_set_charge_period_cmd(sh1107_t const* sh1107,
                                               uint8_t discharge,
                                               uint8_t precharge)
{
    assert(sh1107);

    uint8_t data[2] = {};

    data[0] = SH1107_CMD_SET_CHARGE_PERIOD;
    data[1] = ((discharge & 0x0FU) << 4U) | (precharge & 0x0FU);

    return sh1107_bus_transmit_command(sh1107, data, sizeof(data));
}

sh1107_err_t sh1107_send_set_vcom_deselect_level_cmd(sh1107_t const* sh1107, uint8_t level)
{
    assert(sh1107);

    uint8_t data[2] = {};

    data[0] = SH1107_CMD_SET_VCOM_DESELECT_LEVEL;
    data[1] = level & 0xFFU;

    return sh1107_bus_transmit_command(sh1107, data, sizeof(data));
}

sh1107_err_t sh1107_send_set_display_start_line_cmd(sh1107_t const* sh1107, uint8_t line)
{
    assert(sh1107);

    uint8_t data[2] = {};

    data[0] = SH1107_CMD_SET_DISPLAY_START_LINE;
    data[1] = line & 0x3FU;

    return sh1107_bus_transmit_command(sh1107, data, sizeof(data));
}
