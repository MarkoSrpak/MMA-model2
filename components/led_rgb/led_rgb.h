/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

#ifndef LED_RGB_H
#define LED_RGB_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include <stdint.h>
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define LED_RGB_MAX 9 /*!< Maximum number of RGB LEDs supported */
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/
void led_rgb_init(void);
void led_rgb_set_color(uint32_t ledId, uint32_t r, uint32_t g, uint32_t b);
void led_rgb_set_color_hsv(uint32_t ledId, uint32_t h, uint32_t s, uint32_t v);
void led_rgb_on(uint32_t ledId);
void led_rgb_off(uint32_t ledId);
void led_rgb_set_brightness(uint32_t ledId, uint32_t brightness);

#endif /*LED_RGB_H*/
