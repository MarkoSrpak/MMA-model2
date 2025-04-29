/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

#ifndef LED_STRIP_ENCODER_H
#define LED_STRIP_ENCODER_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "driver/rmt_encoder.h"
#include <stdint.h>
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/

/**
 * @brief Type of led strip encoder configuration
 */
typedef struct {
    uint32_t resolution; /*!< Encoder resolution, in Hz */
} led_strip_encoder_config_t;
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/

/**
 * @brief Create RMT encoder for encoding LED strip pixels into RMT symbols
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating led strip encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_led_strip_encoder(const led_strip_encoder_config_t *config,
                                    rmt_encoder_handle_t *ret_encoder);

/**
 * @brief Simple helper function, converting HSV color space to RGB color
 * space
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 */
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r,
                       uint32_t *g, uint32_t *b);

#endif /*LED_STRIP_ENCODER_H*/
