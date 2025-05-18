/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "pwm.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
static bool pwm_is_on[ID_MAX] = {false};
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/

int pwm_init(int led_id)
{
    int gpio_pin = 0;
    if (led_id == LEDC_CHANNEL_BUZZER) {
        gpio_pin = GPIO_BUZZER;
    }

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,          // isti
        .timer_num = LEDC_TIMER,          // isti
        .duty_resolution = LEDC_DUTY_RES, // isti
        .freq_hz = LEDC_FREQUENCY,        // Set output frequency at 5 kHz
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {.speed_mode = LEDC_MODE,
                                          .channel = led_id,
                                          .timer_sel = LEDC_TIMER,
                                          .intr_type = LEDC_INTR_DISABLE,
                                          .gpio_num = gpio_pin,
                                          .duty = 0, // Set duty to %
                                          .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    return 0;
}

int pwm_on_perc(int led_id, int pwm_percentage)
{
    if (pwm_percentage > 100) {
        pwm_percentage = 100;
    }
    if (pwm_percentage < 0) {
        pwm_percentage = 0;
    }
    int ledc_duty = (LEDC_DUTY * pwm_percentage) / 100;
    ledc_set_duty(LEDC_MODE, led_id, ledc_duty);
    ledc_update_duty(LEDC_MODE, led_id);
    if (pwm_percentage > 0) {
        pwm_is_on[led_id] = true;
    } else {
        pwm_is_on[led_id] = false;
    }
    return 0;
}

int pwm_on(int led_id)
{
    pwm_on_perc(led_id, 100);
    return 0;
}

int pwm_off(int led_id)
{
    pwm_on_perc(led_id, 0);
    return 0;
}

int pwm_toggle(int led_id)
{
    if (pwm_is_on[led_id]) {
        pwm_off(led_id);
    } else {
        pwm_on(led_id);
    }
    return 0;
}
