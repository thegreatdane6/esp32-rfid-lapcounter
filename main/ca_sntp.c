#include <esp_log.h>
#include <esp_event.h>
#include <esp_sntp.h>

#include "ca_sntp.h"

ESP_EVENT_DEFINE_BASE(CA_SNTP_EVENT);

/* logging tag */
static const char *TAG = "sntp";

/* event handler */
static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);

/* callback handler for SNTP */
static void sync_time_cb(struct timeval *tv);

void ca_sntp_setup(void) {
    ESP_LOGD(TAG, "Register event handler for IP_EVENT when *_GOT_IP emmited");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));

    ESP_LOGD(TAG, "Set SNTP operating mode to poll");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    ESP_LOGD(TAG, "Add SNTP servers");
    sntp_setservername(0, "0.pool.ntp.org");
    sntp_setservername(1, "1.pool.ntp.org");
    sntp_setservername(2, "2.pool.ntp.org");
    sntp_setservername(3, "3.pool.ntp.org");

#ifdef CONFIG_ESP32_RFID_LAP_COUNTER_SNTP_TIME_SYNC_METHOD_SMOOTH
    ESP_LOGD(TAG, "Set SNTP sync mode to smooth");
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif

    ESP_LOGD(TAG, "Set SNTP time sync notification callback");
    sntp_set_time_sync_notification_cb(sync_time_cb);
}

static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data) {
    switch (event_id) {
    case IP_EVENT_ETH_GOT_IP:
    case IP_EVENT_PPP_GOT_IP:
    case IP_EVENT_STA_GOT_IP: {
        if (!sntp_restart())
            sntp_init();
        break;
    }
    }
}

static void sync_time_cb(struct timeval *tv) {
    switch (sntp_get_sync_status()) {
    case SNTP_SYNC_STATUS_RESET:
        ESP_LOGD(TAG, "status reset");
        esp_event_post(CA_SNTP_EVENT, CA_SNTP_EVENT_RESET, tv, sizeof(struct timeval), portMAX_DELAY);
        break;

    case SNTP_SYNC_STATUS_COMPLETED: {
#if CONFIG_LOG_DEFAULT_LEVEL >= 4
        char strftime_buf[64];

        time_t now;
        struct tm timeinfo;

        time(&now);
        localtime_r(&now, &timeinfo);

        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGD(TAG, "date/time updated: %s", strftime_buf);
#endif /* CONFIG_BOOTLOADER_LOG_LEVEL */
        esp_event_post(CA_SNTP_EVENT, CA_SNTP_EVENT_COMPLETED, tv, sizeof(struct timeval), portMAX_DELAY);
        break;
    }

    case SNTP_SYNC_STATUS_IN_PROGRESS:
        ESP_LOGD(TAG, "update date/time in progress");
        esp_event_post(CA_SNTP_EVENT, CA_SNTP_EVENT_IN_PROGRESS, tv, sizeof(struct timeval), portMAX_DELAY);
        break;
    }
}