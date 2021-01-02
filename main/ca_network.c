#include <esp_log.h>
#include <esp_event.h>
#include <esp_wifi.h>

#include "ca_network.h"

/* logging tag */
static const char *TAG = "network";

/* logging tag for Wi-Fi feature */
static const char *TAG_WIFI = "network-wifi";

/* event handler */
static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);

void ca_network_setup(void) {
    ESP_LOGD(TAG, "register event handler for listening on network connection & disconnection");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));

    ESP_LOGI(TAG, "initialize TCP/IP stack");
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_LOGI(TAG, "create default Wi-Fi STA");
    esp_netif_create_default_wifi_sta();

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP
    ESP_LOGI(TAG, "create default Wi-Fi AP");
    esp_netif_create_default_wifi_ap();
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP */

    ESP_LOGD(TAG, "create default Wi-Fi configuration used for initialization the Wi-Fi stack");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_LOGI(TAG, "initialize Wi-Fi stack");
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_STA_DISCONNECTED: {
            ESP_LOGI(TAG_WIFI, "disconnected");
            ESP_LOGI(TAG_WIFI, "connect to the AP again");
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
        }

        case WIFI_EVENT_STA_START: {
            ESP_LOGI(TAG_WIFI, "connect to the AP");
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
            break;
        }
        }
    }
}