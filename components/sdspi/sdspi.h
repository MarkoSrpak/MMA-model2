/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

#ifndef SDSPI_H
#define SDSPI_H

/*--------------------------- INCLUDES ---------------------------------------*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdio.h>

/*--------------------------- MACROS AND DEFINES -----------------------------*/
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- EXTERN -----------------------------------------*/
/*--------------------------- GLOBAL FUNCTION PROTOTYPES ---------------------*/
void sdspi_init();
void sdspi_write_line(const char *file_name, char *txBuffer, uint8_t txSize);
void sdspi_read(const char *file_name, char *rxBuffer, uint8_t rxSize);
#endif /*SDSPI_H*/
