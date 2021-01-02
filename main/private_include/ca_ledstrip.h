#ifndef __ESP32_RFID_LAP_COUNTER_LEDSTRIP_H__
#define __ESP32_RFID_LAP_COUNTER_LEDSTRIP_H__

#include <esp_event_base.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum
    {
        CA_LEDSTRIP_EVENT_UPDATE,
        CA_LEDSTRIP_EVENT_RESET,
        CA_LEDSTRIP_EVENT_ANIMATE,
        CA_LEDSTRIP_EVENT_ANIMATE_START,
        CA_LEDSTRIP_EVENT_ANIMATE_STEP,
        CA_LEDSTRIP_EVENT_ANIMATE_FINISH,
    };

    ESP_EVENT_DECLARE_BASE(CA_LEDSTRIP_EVENT);

    typedef enum
    {
        CA_LEDSTRIP_ANIMATION_CONTINUE,
        CA_LEDSTRIP_ANIMATION_RESET,
    } ca_ledstrip_animation_e;

    typedef struct
    {
        uint32_t id;
        uint32_t color;
        uint64_t delay;
        uint32_t span;
        ca_ledstrip_animation_e anim_type;
    } ca_ledstrip_animation_step_t;

    typedef struct
    {
        ca_ledstrip_animation_step_t *steps;
        uint32_t length;
    } ca_ledstrip_animation_t;

    void ca_ledstrip_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* __ESP32_RFID_LAP_COUNTER_LEDSTRIP_H__ */