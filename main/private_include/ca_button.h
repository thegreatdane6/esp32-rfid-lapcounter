#ifndef __ESP32_RFID_LAP_COUNTER_BUTTON_H__
#define __ESP32_RFID_LAP_COUNTER_BUTTON_H__

#include <esp_event_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CA_BUTTON_EVENT_PRESSED,
    CA_BUTTON_EVENT_RELEASED
};

ESP_EVENT_DECLARE_BASE(CA_BUTTON_EVENT);

void ca_button_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* __ESP32_RFID_LAP_COUNTER_BUTTON_H__ */