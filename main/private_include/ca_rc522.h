#ifndef __ESP32_RFID_LAP_COUNTER_RC522_H__
#define __ESP32_RFID_LAP_COUNTER_RC522_H__

#include <esp_event_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CA_RC522_EVENT_UPDATE
};

ESP_EVENT_DECLARE_BASE(CA_RC522_EVENT);

void ca_rc522_setup(void);
#ifdef __cplusplus
}
#endif

#endif /* __ESP32_RFID_LAP_COUNTER_RC522_H__ */