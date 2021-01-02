#ifndef __ESP32_RFID_LAP_COUNTER_BUZZER_H__
#define __ESP32_RFID_LAP_COUNTER_BUZZER_H__

#include <esp_event_base.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum
    {
        CA_BUZZER_EVENT_BEEP,
        CA_BUZZER_EVENT_PERIODIC_BEEP,
        CA_BUZZER_EVENT_START,
        CA_BUZZER_EVENT_END
    };

    ESP_EVENT_DECLARE_BASE(CA_BUZZER_EVENT);

    void ca_buzzer_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* __ESP32_RFID_LAP_COUNTER_BUZZER_H__ */