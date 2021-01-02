#ifndef __ESP32_RFID_LAP_COUNTER_OTA_H__
#define __ESP32_RFID_LAP_COUNTER_OTA_H__

#include <esp_event_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CA_OTA_EVENT_CHECK,
    CA_OTA_EVENT_AVAILABLE,
    CA_OTA_EVENT_UP_TO_DATE,
    CA_OTA_EVENT_FAILED,
    CA_OTA_EVENT_CLEANUP
};

ESP_EVENT_DECLARE_BASE(CA_OTA_EVENT);

void ca_ota_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* __ESP32_RFID_LAP_COUNTER_OTA_H__ */