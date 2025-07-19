#ifndef SH1107_SH1107_H
#define SH1107_SH1107_H

#include "sh1107_commands.h"
#include "sh1107_config.h"
#include <stdbool.h>

typedef struct {
    sh1107_config_t config;
    sh1107_interface_t interface;

    uint8_t frame_buf[SH1107_FRAME_BUF_SIZE];
} sh1107_t;

sh1107_err_t sh1107_initialize(sh1107_t* sh1107,
                               sh1107_config_t const* config,
                               sh1107_interface_t const* interface);
sh1107_err_t sh1107_deinitialize(sh1107_t* sh1107);

sh1107_err_t sh1107_display_frame_buf(sh1107_t const* sh1107);
void sh1107_clear_frame_buf(sh1107_t* sh1107);

sh1107_err_t sh1107_set_pixel(sh1107_t* sh1107,
                              uint8_t x,
                              uint8_t y,
                              bool color);
sh1107_err_t sh1107_draw_line(sh1107_t* sh1107,
                              uint8_t x0,
                              uint8_t y0,
                              uint8_t x1,
                              uint8_t y1,
                              bool color);
sh1107_err_t sh1107_draw_rect(sh1107_t* sh1107,
                              uint8_t x,
                              uint8_t y,
                              uint8_t w,
                              uint8_t h,
                              bool color);
sh1107_err_t sh1107_draw_circle(sh1107_t* sh1107,
                                uint8_t x0,
                                uint8_t y0,
                                uint8_t r,
                                bool color);
sh1107_err_t sh1107_draw_bitmap(sh1107_t* sh1107,
                                uint8_t x,
                                uint8_t y,
                                uint8_t w,
                                uint8_t h,
                                uint8_t* bitmap,
                                size_t bitmap_size,
                                bool color);
sh1107_err_t sh1107_draw_char(sh1107_t* sh1107, uint8_t x, uint8_t y, char c);
sh1107_err_t sh1107_draw_string(sh1107_t* sh1107,
                                uint8_t x,
                                uint8_t y,
                                char const* s);
sh1107_err_t sh1107_draw_string_formatted(sh1107_t* sh1107,
                                          uint8_t x,
                                          uint8_t y,
                                          char const* fmt,
                                          ...);

sh1107_err_t sh1107_device_reset(sh1107_t const* sh1107);

sh1107_err_t sh1107_send_set_lower_column_address_cmd(sh1107_t const* sh1107,
                                                      uint8_t address);
sh1107_err_t sh1107_send_set_higher_column_address_cmd(sh1107_t const* sh1107,
                                                       uint8_t address);
sh1107_err_t sh1107_send_set_memory_addressing_mode_cmd(sh1107_t const* sh1107,
                                                        uint8_t mode);
sh1107_err_t sh1107_send_set_segment_remap_cmd(sh1107_t const* sh1107,
                                               uint8_t remap);
sh1107_err_t sh1107_send_set_entire_display_on_off_cmd(sh1107_t const* sh1107,
                                                       uint8_t on_off);
sh1107_err_t sh1107_send_set_normal_reverse_display_cmd(sh1107_t const* sh1107,
                                                        uint8_t display);
sh1107_err_t sh1107_send_set_display_on_off_cmd(sh1107_t const* sh1107,
                                                uint8_t on_off);
sh1107_err_t sh1107_send_set_page_address_cmd(sh1107_t const* sh1107,
                                              uint8_t address);
sh1107_err_t sh1107_send_set_output_scan_direction_cmd(sh1107_t const* sh1107,
                                                       uint8_t direction);

sh1107_err_t sh1107_send_read_modify_write_cmd(sh1107_t const* sh1107);
sh1107_err_t sh1107_send_end_cmd(sh1107_t const* sh1107);
sh1107_err_t sh1107_send_nop_cmd(sh1107_t const* sh1107);
sh1107_err_t sh1107_send_read_id_cmd(sh1107_t const* sh1107,
                                     uint8_t busy,
                                     uint8_t on_off,
                                     uint8_t id);

sh1107_err_t sh1107_send_set_contrast_control_cmd(sh1107_t const* sh1107,
                                                  uint8_t contrast);
sh1107_err_t sh1107_send_set_multiplex_ratio_cmd(sh1107_t const* sh1107,
                                                 uint8_t ratio);
sh1107_err_t sh1107_send_set_display_offset_cmd(sh1107_t const* sh1107,
                                                uint8_t offset);
sh1107_err_t sh1107_send_set_dc_dc_setting_cmd(sh1107_t const* sh1107,
                                               uint8_t setting);
sh1107_err_t sh1107_send_set_display_clock_cmd(sh1107_t const* sh1107,
                                               uint8_t osc_freq,
                                               uint8_t clock_divide);
sh1107_err_t sh1107_send_set_charge_period_cmd(sh1107_t const* sh1107,
                                               uint8_t discharge,
                                               uint8_t precharge);
sh1107_err_t sh1107_send_set_vcom_deselect_level_cmd(sh1107_t const* sh1107,
                                                     uint8_t level);
sh1107_err_t sh1107_send_set_display_start_line_cmd(sh1107_t const* sh1107,
                                                    uint8_t line);

#endif // SH1107_SH1107_H