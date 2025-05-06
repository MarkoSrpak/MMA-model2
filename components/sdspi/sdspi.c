/**
 * MMA model2
 * Lumen Engineering 2025
 *
 */

/*--------------------------- INCLUDES ---------------------------------------*/
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
/*--------------------------- MACROS AND DEFINES -----------------------------*/
#define PIN_NUM_MOSI  36
#define PIN_NUM_MISO  35
#define PIN_NUM_CLK   37
#define PIN_NUM_SD_CS 38
#define MOUNT_POINT   "/sdcard"

// DO NOT CHANGE THE FOLLOWING LINE, WE DONT WANT TO FORMAT THE SD CARD
#define FORMAT_IF_MOUNT_FAILED 0
/*--------------------------- TYPEDEFS AND STRUCTS ---------------------------*/
/*--------------------------- STATIC FUNCTION PROTOTYPES ---------------------*/
/*--------------------------- VARIABLES --------------------------------------*/
static const char *TAG = "sdspi";
/*--------------------------- STATIC FUNCTIONS -------------------------------*/
/*--------------------------- GLOBAL FUNCTIONS -------------------------------*/

void sdspi_init()
{
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#if FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT
    // filesystem. Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience
    // functions. Please check its source code and implement error recovery when
    // developing production applications.
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP)
    // signals. Modify slot_config.gpio_cd and slot_config.gpio_wp if your board
    // has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_SD_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config,
                                  &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(
                TAG,
                "Failed to mount filesystem. "
                "If you want the card to be formatted, set the "
                "CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG,
                     "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Only few folders will be made, so we can have them hardcoded in init
    const char *log_dir = MOUNT_POINT "/logs";
    struct stat st = {0};

    // Create directory if it doesn't exist
    if (stat(log_dir, &st) == -1) {
        ESP_LOGI(TAG, "Creating directory: %s", log_dir);
        if (mkdir(log_dir, 0775) != 0) {
            ESP_LOGE(TAG, "Failed to create directory (%d): %s", errno,
                     strerror(errno));
        }
    }
}

void sdspi_write_line(const char *file_name, char *txBuffer, uint8_t txSize)
{
    char full_path[64];
    snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, file_name);

    ESP_LOGI(TAG, "Opening file %s", full_path);
    FILE *f = fopen(full_path, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for apending");
        return;
    }
    // Ensure line ends with a newline character
    if (txSize > 0 && txBuffer[txSize - 1] != '\n') {
        fprintf(f, "%.*s\n", txSize, txBuffer);
    } else {
        fprintf(f, "%s", txBuffer);
    }

    ESP_LOGI(TAG, "Wrote to file: '%.*s'", txSize, txBuffer);
    fclose(f);
}

void sdspi_read(const char *file_name, char *rxBuffer, uint8_t rxSize)
{
    char full_path[64];
    snprintf(full_path, sizeof(full_path), "%s/%s", MOUNT_POINT, file_name);

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file %s", full_path);
    FILE *f = fopen(full_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        rxBuffer[0] = '\0'; // Ensure rxBuffer is a valid empty string
        return;
    }
    // Read a line from file
    char line[128];
    if (fgets(line, sizeof(line), f) != NULL) {
        // Strip newline
        char *pos = strchr(line, '\n');
        if (pos) {
            *pos = '\0';
        }

        // Copy to rxBuffer with safety
        strncpy(rxBuffer, line, rxSize - 1);
        rxBuffer[rxSize - 1] = '\0'; // Ensure null-termination
        ESP_LOGI(TAG, "Read from file: '%s'", rxBuffer);
    } else {
        rxBuffer[0] = '\0'; // In case of read error
        ESP_LOGE(TAG, "Failed to read from file");
    }

    fclose(f);
}
