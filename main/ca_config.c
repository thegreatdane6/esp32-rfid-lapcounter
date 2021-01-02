#include <esp_log.h>
#include <esp_event.h>
#include <nvs.h>
#include <nvs_flash.h>

#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

#include "ca_config.h"

ESP_EVENT_DEFINE_BASE(CA_CONFIG_EVENT);

/* logging tag */
static const char *TAG = "config";

void ca_config_setup(void) {
    nvs_handle_t ns_system;

    ESP_LOGI(TAG, "open 'system' NVS namespace");
    ESP_ERROR_CHECK(nvs_open("system", NVS_READWRITE, &ns_system));

    /* time & timezone block */
    {
        esp_err_t err;
        int c_err;
        char *timezone = NULL;
        size_t len = 0;

        /* grab the required length */
        if ((err = nvs_get_str(ns_system, "tz", NULL, &len)) == ESP_OK) {
            timezone = calloc(1, len);

            if (timezone == NULL) {
                ESP_LOGE(TAG, "allocation failed: %s", strerror(errno));
                abort();
            }

            if ((err = nvs_get_str(ns_system, "tz", timezone, &len)) == ESP_OK) {
                ESP_LOGI(TAG, "timezone loaded from configuration: %s", timezone);
            }
        }

        /* the previous calls failed, set the default timezone */
        if (err != ESP_OK) {
            /* free memory before we set the default one */
            if (timezone != NULL) {
                free(timezone);
            }

            timezone = CONFIG_ESP32_RFID_LAP_COUNTER_TIMEZONE_DEFAULT;
        }

        if ((c_err = setenv("TZ", timezone, 1)) != 0) {
            ESP_LOGE(TAG, "unable to set TZ environment: %s", strerror(errno));

            switch (c_err) {
            case ENOMEM:
                abort();

            case EINVAL: {
                /* remove tz because value is invalid */
                if (err == ESP_OK) {
                    nvs_erase_key(ns_system, "tz");
                }
            }
            }
        } else {
            tzset();
        }

#ifndef CONFIG_ESP32_RFID_LAP_COUNTER_SNTP_ENABLED
        uint64_t tv_sec, tv_usec;
        if ((err = nvs_get_u64(ns_system, "tv_sec", &tv_sec)) == ESP_OK && (err = nvs_get_u64(ns_system, "tv_usec", &tv_usec) == ESP_OK)) {
            struct timeval tv = {
                .tv_sec = tv_sec,
                .tv_usec = tv_usec,
            };

            if ((c_err = settimeofday(&tv, NULL)) != 0) {
                ESP_LOGE(TAG, "unable to set time of the day: %s", strerror(errno));

                switch (c_err) {
                case ENOMEM:
                case EFAULT:
                case EPERM:
                    abort();

                case EINVAL: {
                    /* remove tv_sec and tv_usec because are invalid */
                    nvs_erase_key(ns_system, "tv_sec");
                    nvs_erase_key(ns_system, "tv_usec");
                }
                }
            }
        }
#endif
    }
}