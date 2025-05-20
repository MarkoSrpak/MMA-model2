/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gps.h"
#include "gps_task.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define TIME_ZONE (+2)   // Beijing Time
#define YEAR_BASE (2000) // date in GPS starts from 2000
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
static void gps_event_handler(void *event_handler_arg,
                              esp_event_base_t event_base, int32_t event_id,
                              void *event_data);
/*--------------------------- VARIABLES --------------------------------------*/
static const char *TAG = "gps_task";
/*--------------------------- STATIC FUNCTIONS -------------------------------*/

/**
 * @brief GPS Event Handler
 *
 * @param event_handler_arg handler specific arguments
 * @param event_base event base, here is fixed to ESP_NMEA_EVENT
 * @param event_id event id
 * @param event_data event specific arguments
 */
static void gps_event_handler(void *event_handler_arg,
                              esp_event_base_t event_base, int32_t event_id,
                              void *event_data)
{
    gps_t *gps = NULL;
    switch (event_id) {
    case GPS_UPDATE :
        // ESP_LOGI(TAG, "Raw info: %s Size: %d", (char *)event_data,
        // strlen(event_data));
        gps = (gps_t *)event_data;
        /* print information parsed from GPS statements */
        ESP_LOGI(TAG,
                 "%d/%d/%d %d:%d:%d => \r\n"
                 "\t\t\t\t\t\tlatitude   = %.05f°N\r\n"
                 "\t\t\t\t\t\tlongitude = %.05f°E\r\n"
                 "\t\t\t\t\t\taltitude   = %.02fm\r\n"
                 "\t\t\t\t\t\tspeed      = %fm/s",
                 gps->date.year + YEAR_BASE, gps->date.month, gps->date.day,
                 gps->tim.hour + TIME_ZONE, gps->tim.minute, gps->tim.second,
                 gps->latitude, gps->longitude, gps->altitude, gps->speed);
        break;
    case GPS_UNKNOWN :
        /* print unknown statements */
        //  ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
        break;
    default :
        break;
    }
}
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void gps_task(void *pvParameters)
{
    /* NMEA parser configuration */
    nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT();
    /* init NMEA parser library */
    nmea_parser_handle_t nmea_hdl = nmea_parser_init(&config);
    /* register event handler for NMEA parser library */
    nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);

    // this task never does anything
    while (1) {
        vTaskDelay(600000 / portTICK_PERIOD_MS);
    }

    /* unregister event handler */
    nmea_parser_remove_handler(nmea_hdl, gps_event_handler);
    /* deinit NMEA parser library */
    nmea_parser_deinit(nmea_hdl);
}
