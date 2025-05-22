/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "led_rgb.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "led_strip_encoder.h"
#include <string.h>
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define RMT_LED_STRIP_GPIO_NUM      5
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
static const char *TAG = "led_rgb";
static uint8_t led_strip_pixels[LED_RGB_MAX * 3]; // RGB values for each LED
static rmt_channel_handle_t led_chan = NULL;
static rmt_encoder_handle_t led_encoder = NULL;
static rmt_transmit_config_t tx_config = {
    .loop_count = 0, // No transfer loop
};
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/

void led_rgb_init(void)
{
    ESP_LOGI(TAG, "Initializing RGB LED strip");

    // Configure RMT TX channel
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64,
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4,
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    // Configure LED strip encoder
    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    // Enable RMT TX channel
    ESP_ERROR_CHECK(rmt_enable(led_chan));

    // Clear LED strip
    memset(led_strip_pixels, 0, sizeof(led_strip_pixels));
    ESP_LOGI(TAG, "RGB LED strip initialized");

    for (int i = 0; i < LED_RGB_MAX; i++) {
        led_rgb_off(i); // Set all LEDs to off
    }
}

void led_rgb_set_color(uint32_t ledId, uint32_t r, uint32_t g, uint32_t b)
{
    if (ledId >= LED_RGB_MAX) {
        ESP_LOGE(TAG, "Invalid LED ID: %ld", ledId);
        return;
    }

    // Set RGB values for the specified LED
    led_strip_pixels[ledId * 3 + 0] = r; // g; // Green
    led_strip_pixels[ledId * 3 + 1] = g; // b; // Blue
    led_strip_pixels[ledId * 3 + 2] = b; // r; // Red

    // ESP_LOGI(TAG, "Set LED %ld color to R:%ld G:%ld B:%ld", ledId, r, g, b);

    // Transmit the current LED strip pixel values
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels,
                                 sizeof(led_strip_pixels), &tx_config));
    // Wait for transmission to complete, not necessary but could be a problem
    // if leds are updated too fast
    // ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
    // ESP_LOGI(TAG, "Flushed LED values to strip");
}

void led_rgb_set_color_hsv(uint32_t ledId, uint32_t h, uint32_t s, uint32_t v)
{
    if (ledId >= LED_RGB_MAX) {
        ESP_LOGE(TAG, "Invalid LED ID: %ld", ledId);
        return;
    }

    uint32_t r, g, b;
    led_strip_hsv2rgb(h, s, v, &r, &g, &b);
    led_rgb_set_color(ledId, r, g, b);

    // ESP_LOGI(TAG, "Set LED %ld color to HSV H:%ld S:%ld V:%ld", ledId, h, s,
    // v);
}

void led_rgb_on(uint32_t ledId)
{
    if (ledId >= LED_RGB_MAX) {
        ESP_LOGE(TAG, "Invalid LED ID: %ld", ledId);
        return;
    }

    // Turn on the LED by setting it to white (full brightness)
    led_rgb_set_color(ledId, 255, 255, 255);
    // ESP_LOGI(TAG, "Turned on LED %ld", ledId);
}

void led_rgb_off(uint32_t ledId)
{
    if (ledId >= LED_RGB_MAX) {
        ESP_LOGE(TAG, "Invalid LED ID: %ld", ledId);
        return;
    }

    // Turn off the LED by setting RGB values to 0
    led_rgb_set_color(ledId, 0, 0, 0);
    // ESP_LOGI(TAG, "Turned off LED %ld", ledId);
}

void led_rgb_set_brightness(uint32_t ledId, uint32_t brightness)
{
    if (ledId >= LED_RGB_MAX || brightness > 100) {
        ESP_LOGE(TAG, "Invalid LED ID or brightness: %ld, %ld", ledId,
                 brightness);
        return;
    }

    // Scale the current RGB values by the brightness
    uint32_t r = led_strip_pixels[ledId * 3 + 2];
    uint32_t g = led_strip_pixels[ledId * 3 + 0];
    uint32_t b = led_strip_pixels[ledId * 3 + 1];

    r = (r * brightness) / 100;
    g = (g * brightness) / 100;
    b = (b * brightness) / 100;

    led_rgb_set_color(ledId, r, g, b);
    ESP_LOGI(TAG, "Set LED %ld brightness to %ld", ledId, brightness);
}
