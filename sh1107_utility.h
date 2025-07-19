#ifndef SH1107_SH1107_UTILITY_H
#define SH1107_SH1107_UTILITY_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

inline bool sh1107_bitmap_get_pixel(uint8_t width,
                                    uint8_t height,
                                    uint8_t (*bitmap)[width * height / 8],
                                    uint8_t x,
                                    uint8_t y)
{
    assert(bitmap);

    uint8_t byte_num = (y * width + x) / 8U;
    uint8_t bit_mask = 1U << (7U - (x % 8U));

    bool pixel = (*bitmap[byte_num] & bit_mask) ? true : false;

    return pixel;
}

inline void sh1107_bitmap_set_pixel(uint8_t width,
                                    uint8_t height,
                                    uint8_t (*bitmap)[width * height / 8],
                                    uint8_t x,
                                    uint8_t y,
                                    bool pixel)
{
    assert(bitmap);

    uint8_t byte_num = (y * width + x) / 8U;
    uint8_t bit_mask = 1U << (7U - (x % 8U));

    *bitmap[byte_num] = pixel ? (*bitmap[byte_num] | bit_mask)
                              : (*bitmap[byte_num] & ~bit_mask);
}

inline void sh1107_bitmap_resize(
    uint8_t old_width,
    uint8_t old_height,
    uint8_t (*old_bitmap)[old_width * old_height / 8],
    uint8_t new_width,
    uint8_t new_height,
    uint8_t (*new_bitmap)[new_width * new_height / 8])
{
    assert(old_bitmap && new_bitmap);

    for (uint8_t new_y = 0U; new_y < new_height; ++new_y) {
        for (uint8_t new_x = 0U; new_x < new_width; ++new_x) {
            uint8_t old_x = new_x * old_width / new_width;
            uint8_t old_y = new_y * old_height / new_height;

            bool pixel = sh1107_bitmap_get_pixel(old_width,
                                                 old_height,
                                                 old_bitmap,
                                                 old_x,
                                                 old_y);
            sh1107_bitmap_set_pixel(new_width,
                                    new_height,
                                    new_bitmap,
                                    new_x,
                                    new_y,
                                    pixel);
        }
    }
}

#endif // SH1107_SH1107_UTILITY_H