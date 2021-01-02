#ifndef __ESP32_RFID_LAP_COUNTER_CONFIG_H__
#define __ESP32_RFID_LAP_COUNTER_CONFIG_H__

#include <esp_event_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CA_CONFIG_EVENT_UPDATE
};

ESP_EVENT_DECLARE_BASE(CA_CONFIG_EVENT);

void ca_config_setup(void);

// esp_err_t ca_config_set_i8(const char *ns, const char *key, int8_t value);
// esp_err_t ca_config_set_u8(const char *ns, const char *key, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* __ESP32_RFID_LAP_COUNTER_CONFIG_H__ */