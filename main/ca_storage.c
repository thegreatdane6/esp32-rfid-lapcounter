#include <esp_log.h>
#include <esp_event.h>
#include <esp_vfs.h>
#include <esp_vfs_fat.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "ca_storage.h"

/* logging tag */
static const char *TAG = "storage";

/* NVS flash setup */
static void nvs_setup(void);

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_FATFS_ENABLED
/* FAT partition setup */
static void fat_setup(void);

/* handle of the wear levelling library instance */
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_FATFS_ENABLED */

void ca_storage_setup(void) {
    ESP_LOGI(TAG, "initialize NVS storage");
    nvs_setup();

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_FATFS_ENABLED
    ESP_LOGI(TAG, "initialize FAT storage");
    fat_setup();
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_FATFS_ENABLED */
}

static void nvs_setup(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS storage initialization failed %s", esp_err_to_name(err));
        ESP_LOGW(TAG, "erase NVS storage");
        ESP_ERROR_CHECK(nvs_flash_erase());

        ESP_LOGI(TAG, "initialize NVS storage");
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_FATFS_ENABLED
static void fat_setup(void) {
    /* Mount configuration */
    const esp_vfs_fat_mount_config_t s_mount_config = {
        .max_files = 4,
        .format_if_mount_failed = true,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE};

    ESP_LOGD(TAG, "mount 'storage' partition as /data");
    ESP_ERROR_CHECK(esp_vfs_fat_spiflash_mount("/data", "storage", &s_mount_config, &s_wl_handle));
}
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_FATFS_ENABLED */