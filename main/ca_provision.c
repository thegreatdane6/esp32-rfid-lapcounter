#include <esp_log.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_bt.h>

#include <wifi_provisioning/manager.h>
#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE
#include <wifi_provisioning/scheme_ble.h>
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE */

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP
#include <wifi_provisioning/scheme_softap.h>
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP */

#include "ca_provision.h"

/* device capabilities */
static const char *DEVICE_CAPABILITIES[] = {};

/* logging tag */
static const char *TAG = "provision";

/* event handler */
static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);

void ca_provision_setup(void) {
    bool provisioned = false;

    ESP_LOGD(TAG, "register event handler for WIFI_PROV_EVENT");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE
    ESP_LOGI(TAG, "BLE scheme selected for provision manager");
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE */

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP
    ESP_LOGI(TAG, "Wi-Fi SoftAP scheme selected for provision manager");
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP */

    wifi_prov_mgr_config_t config = {
#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE
        .scheme = wifi_prov_scheme_ble,
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE */
#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP
        .scheme = wifi_prov_scheme_softap,
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP */
#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE */
#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_SOFTAP */
    };

    ESP_LOGI(TAG, "initialize provision manager");
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    ESP_LOGI(TAG, "check if device is provisioned");
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        char service_name[sizeof(CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_NAME_PREFIX) + 6] = {0};
        char proof_of_possesion[sizeof(CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_NAME_PREFIX) + 6] = {0};
#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_KEY_ENABLED
#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_KEY_DYNAMIC
        char service_key[9] = {0};
#endif
#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_KEY_STATIC
        char service_key[] = CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_KEY_STATIC_VALUE;
#endif
#else
        const char *service_key = NULL;
#endif
        uint8_t wifi_mac[6] = {0};
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        ESP_LOGD(TAG, "set application info for provision manager");
        wifi_prov_mgr_set_app_info(CONFIG_LWIP_LOCAL_HOSTNAME, CONFIG_APP_PROJECT_VER, DEVICE_CAPABILITIES, sizeof(DEVICE_CAPABILITIES) / sizeof(DEVICE_CAPABILITIES[0]));

        ESP_LOGD(TAG, "get Wi-Fi STA MAC address");
        ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, wifi_mac));

        ESP_LOGD(TAG, "format service name with prefix " CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_NAME_PREFIX " and parts of the MAC address");
        snprintf(service_name, sizeof(service_name), CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_NAME_PREFIX "%02X%02X%02X", wifi_mac[3], wifi_mac[4], wifi_mac[5]);

        ESP_LOGD(TAG, "format proof of possesion with prefix " CONFIG_ESP32_RFID_LAP_COUNTER_PROV_PROOF_OF_POSSESION_PREFIX " and parts of the MAC address");
        snprintf(proof_of_possesion, sizeof(proof_of_possesion), CONFIG_ESP32_RFID_LAP_COUNTER_PROV_PROOF_OF_POSSESION_PREFIX "%02x%02x%02x", wifi_mac[3], wifi_mac[4], wifi_mac[5]);

#if CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_KEY_ENABLED && CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_KEY_DYNAMIC
        ESP_LOGD(TAG, "format service key using random values");
        snprintf(service_key, sizeof(service_key), "%04x%04x", esp_random(), esp_random());
#endif

        ESP_LOGI(TAG, "device service name: %s", service_name);
        ESP_LOGI(TAG, "proof-of-possesion: %s", proof_of_possesion);
#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_SERVICE_KEY_ENABLED
        ESP_LOGI(TAG, "service key (for Wi-Fi): %s", service_key);
#endif

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE
        uint8_t custom_service_uuid[] = {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4,
            0xdf,
            0x5a,
            0x1c,
            0x3f,
            0x6b,
            0xf4,
            0xbf,
            0xea,
            0x4a,
            0x82,
            0x03,
            0x04,
            0x90,
            0x1a,
            0x02,
        };
        wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
#endif /* CONFIG_ESP32_RFID_LAP_COUNTER_PROV_TRANSPORT_BLE */

        ESP_LOGI(TAG, "start provisioning");
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, proof_of_possesion, service_name, service_key));
    } else {
        ESP_LOGI(TAG, "device already provisioned");

        ESP_LOGI(TAG, "release provision manager resources");
        wifi_prov_mgr_deinit();

        ESP_LOGD(TAG, "set Wi-Fi mode to STA");
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

        ESP_LOGI(TAG, "start Wi-Fi in STA mode");
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}

static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data) {
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "provisioning started");
            break;
        case WIFI_PROV_CRED_RECV: {
            wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
            ESP_LOGI(TAG, "received Wi-Fi credentials"
                          "\n\tSSID     : %s\n\tPassword : %s",
                     (const char *)wifi_sta_cfg->ssid,
                     (const char *)wifi_sta_cfg->password);
            break;
        }
        case WIFI_PROV_CRED_FAIL: {
            wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
            ESP_LOGE(TAG, "provisioning failed! %s",
                     (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");

            if (reason == WIFI_PROV_STA_AUTH_ERROR) {
                ESP_LOGD(TAG, "erase NVS flash due to invalid Wi-Fi authentication credentials");
                wifi_config_t wifi_cfg = {0};
                esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg);

                ESP_LOGD(TAG, "reboot device");
                esp_restart();
            }
            break;
        }
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "provisioning successful");
            break;
        case WIFI_PROV_END:
            /* De-initialize manager once provisioning is finished */
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
        }
    }
}