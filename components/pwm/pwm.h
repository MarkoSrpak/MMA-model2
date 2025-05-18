/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

#ifndef PWM_H
#define PWM_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "driver/ledc.h"
#include "esp_err.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define GPIO_BUZZER 4

#define LEDC_TIMER          LEDC_TIMER_0        // isti timer za sve
#define LEDC_MODE           LEDC_LOW_SPEED_MODE // isti svima
#define LEDC_CHANNEL_BUZZER LEDC_CHANNEL_0      // led_id

#define ID_MAX    1
#define ID_BUZZER 0

#define LEDC_DUTY_RES  LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY      (8191) // ((2 ** 13) - 1) = 8191 full cycle , 4095 is 50%
#define LEDC_FREQUENCY (5000) // Frequency in Hertz. Set frequency at 5 kHz
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/
int pwm_init(int pwm_id);
int pwm_on_perc(int led_id, int pwm_percentage);
int pwm_on(int led_id);
int pwm_off(int led_id);
int pwm_toggle(int led_id);

#endif /*PWM_H*/
