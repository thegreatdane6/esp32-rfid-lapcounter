#ifndef __ESP32_RFID_LAP_COUNTER_RACE_DIRECTOR_H__
#define __ESP32_RFID_LAP_COUNTER_RACE_DIRECTOR_H__

#include <esp_event_base.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum
    {
        CA_RACE_DIRECTOR_EVENT_TRACK_REGISTER,
        CA_RACE_DIRECTOR_EVENT_TRACK_UNREGISTER,
        CA_RACE_DIRECTOR_EVENT_START_RACE,
    };

    ESP_EVENT_DECLARE_BASE(CA_RACE_DIRECTOR_EVENT);

    void ca_race_director_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* __ESP32_RFID_LAP_COUNTER_RACE_DIRECTOR_H__ */