#ifndef __ESP32_RFID_LAP_COUNTER_SNTP_H__
#define __ESP32_RFID_LAP_COUNTER_SNTP_H__

#include <esp_event_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CA_SNTP_EVENT_RESET,
    CA_SNTP_EVENT_COMPLETED,
    CA_SNTP_EVENT_IN_PROGRESS,
};

ESP_EVENT_DECLARE_BASE(CA_SNTP_EVENT);

void ca_sntp_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* __ESP32_RFID_LAP_COUNTER_SNTP_H__ */