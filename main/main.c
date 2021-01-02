#include <esp_log.h>
#include <esp_event.h>
#include <driver/gpio.h>

#include "ca_config.h"
#include "ca_network.h"
#include "ca_provision.h"
#include "ca_storage.h"
#include "ca_ota.h"
#include "ca_sntp.h"
#include "ca_rc522.h"
#include "ca_button.h"
#include "ca_buzzer.h"
#include "ca_ledstrip.h"

/* logging tag */
static const char *TAG = "init";

void app_main(void) {
    ESP_LOGI(TAG, "install gpio isr service");
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    ESP_LOGI(TAG, "initialize storage");
    ca_storage_setup();

    ESP_LOGI(TAG, "initialize event loop");
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "initialize configuration");
    ca_config_setup();

    ESP_LOGI(TAG, "initialize networking");
    ca_network_setup();

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_SNTP_ENABLED
    ESP_LOGI(TAG, "initialize SNTP");
    ca_sntp_setup();
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_SNTP_ENABLED */

    ESP_LOGI(TAG, "initialize provisioning");
    ca_provision_setup();

    ESP_LOGI(TAG, "initialize OTA");
    // ca_ota_setup();

    ESP_LOGI(TAG, "initialize RC522");
    ca_rc522_setup();

    ESP_LOGI(TAG, "initialize button");
    ca_button_setup();

    ESP_LOGI(TAG, "initialize buzzer");
    ca_buzzer_setup();

    ESP_LOGI(TAG, "initialize led strip");
    ca_ledstrip_setup();
}