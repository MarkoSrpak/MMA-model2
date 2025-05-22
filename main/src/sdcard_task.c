/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

#include "data_queues.h"
#include "sdcard_task.h"
#include "sdspi.h"
/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
static char random_letter();
static void generate_random_filename(char *filename, size_t size);
/*--------------------------- VARIABLES --------------------------------------*/
static bool logging_active = false;
static bool logging_paused = false;
static char current_log_filename[64] = {0};
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
static char random_letter()
{
    return '0' + (esp_random() % 10);
}

// Generate random filename: logs/workoutXXXXXX.txt
static void generate_random_filename(char *filename, size_t size)
{
    char suffix[4];
    for (int i = 0; i < 3; i++) {
        suffix[i] = random_letter();
    }
    suffix[3] = '\0';

    snprintf(filename, size, "logs/wo%s.txt", suffix);
}
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/
void sdcard_task(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(2000));
    // Initialize the SD card
    sdspi_init();

    // Example usage of writing and reading from the SD card
    char txBuffer[128] = "SD Card started";
    char rxBuffer[128];

    // sdspi_write_line("logs/logstart.txt", txBuffer, sizeof(txBuffer));
    // sdspi_read("logs/logstart.txt", rxBuffer, sizeof(rxBuffer));
    sdspi_write_line("logs/wotlu.txt", txBuffer, sizeof(txBuffer));
    sdspi_read("logs/wotlu.txt", rxBuffer, sizeof(rxBuffer));
    printf("Read from SD Card: %s\n", rxBuffer);
    // clang-format off
    char *header = "ts_ms,year,mon,day,hr,min,sec,lat,lon,speed,alt,temp_c,"
                     "pressure_pa,humidity_pct,voc_ohm,noise_energy,sweat_level,"
                     "accel_freq,accel_ampl,accel_dir\n";
    //sdspi_write_line("logs/log.csv", header, strlen(header));
    // clang-format on
    gps_data_t gps_data = {0};
    bme_data_t bme_data = {0};
    mic_data_t mic_data = {0};
    sweat_data_t sweat_data = {0};
    accel_queue_data_t accel_data = {0};

    char logBuffer[512];

    while (1) {
        uint8_t cmd = 0;
        if (xQueueReceive(sdcard_queue, &cmd, 0)) {
            vTaskDelay(pdMS_TO_TICKS(100));
            if (cmd == LOG_CMD_START) {
                if (!logging_active) {
                    // Generate new file name
                    generate_random_filename(current_log_filename,
                                             sizeof(current_log_filename));
                    sdspi_write_line(current_log_filename, header,
                                     strlen(header));
                    logging_active = true;
                    logging_paused = false;
                    printf("Started new log file: %s\n", current_log_filename);
                } else if (logging_active && logging_paused) {
                    logging_paused = false;
                    printf("Resumed logging to: %s\n", current_log_filename);
                }
            } else if (cmd == LOG_CMD_PAUSE) {
                if (logging_active && !logging_paused) {
                    logging_paused = true;
                    printf("Logging paused.\n");
                }
            } else if (cmd == LOG_CMD_STOP) {
                if (logging_active) {
                    logging_active = false;
                    logging_paused = false;
                    current_log_filename[0] = 0;
                    printf("Logging stopped.\n");
                }
            }
        }

        if (logging_active && !logging_paused) {
            // Read data and log
            xQueuePeek(gps_queue, &gps_data, 0);
            xQueuePeek(bme_queue, &bme_data, 0);
            xQueuePeek(mic_queue, &mic_data, 0);
            xQueuePeek(sweat_queue, &sweat_data, 0);
            xQueuePeek(accel_queue, &accel_data, 0);

            // clang-format off
            int len = snprintf(logBuffer, sizeof(logBuffer),
                "%lu,%u,%u,%u,%u,%u,%u,%.5f,%.5f,%.2f,%.2f,%d,%lu,%lu,%lu,%lu,%u,%.2f,%.2f,%llu,%u\n",
                gps_data.timestamp_ms,
                gps_data.year, gps_data.month, gps_data.day,
                gps_data.hour, gps_data.minute, gps_data.second,
                gps_data.latitude, gps_data.longitude, gps_data.speed_mps, gps_data.altitude_m,
                bme_data.temperature_c, bme_data.pressure_pa, bme_data.humidity_pct, bme_data.voc_ohm,
                mic_data.noise_energy,
                sweat_data.sweat_level,
                accel_data.freq, accel_data.ampl, accel_data.accel_energy, accel_data.direction
            );
            // clang-format on
            if (len > 0 && len < sizeof(logBuffer)) {
                sdspi_write_line(current_log_filename, logBuffer, len);
            } else {
                printf("Log formatting error.\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
