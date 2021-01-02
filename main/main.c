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
#include "ca_race_director.h"

/* logging tag */
static const char *TAG = "init";

static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);

void app_main(void)
{
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

    ESP_LOGI(TAG, "initialize race director");
    ca_race_director_setup();

    // TEST
    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_RC522_EVENT, CA_RC522_EVENT_TRIGGER, event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_BUTTON_EVENT, CA_BUTTON_EVENT_RELEASED, event_handler, NULL, NULL));
}

static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data)
{
    if (event_base == CA_RC522_EVENT && event_id == CA_RC522_EVENT_TRIGGER)
        ESP_ERROR_CHECK(esp_event_post(CA_BUZZER_EVENT, CA_BUZZER_EVENT_BEEP, NULL, NULL, portMAX_DELAY));

    if (event_base == CA_BUTTON_EVENT && event_id == CA_BUTTON_EVENT_RELEASED)
        ESP_ERROR_CHECK(esp_event_post(CA_RACE_DIRECTOR_EVENT, CA_RACE_DIRECTOR_EVENT_START_RACE, NULL, NULL, portMAX_DELAY));
}