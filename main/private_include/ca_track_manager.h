#ifndef __ESP32_RFID_LAP_COUNTER_TRACK_MANAGER_H__
#define __ESP32_RFID_LAP_COUNTER_TRACK_MANAGER_H__

#include <esp_event_base.h>
#include <esp_err.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *key;
    const char *value;
} ca_vehicle_owner_stats;

typedef struct {
    uint8_t *id;
    const char *name;
    ca_vehicle_owner_stats *stats;
} ca_vehicle_owner;

typedef struct {
    uint8_t *tag;
    const char *name;
    ca_vehicle_owner *owner;
} ca_vehicle;

typedef struct {
    uint8_t *id;
    const char *name;
    ca_vehicle **vehicles;
    ca_vehicle_owner *winner;
} ca_track;

enum {
    CA_TRACK_MANAGER_EVENT_TRACK_REGISTER,
    CA_TRACK_MANAGER_EVENT_TRACK_UNREGISTER
};

ESP_EVENT_DECLARE_BASE(CA_TRACK_MANAGER_EVENT);

void ca_track_manager_setup(void);
esp_err_t ca_track_manager_register(ca_track *);
esp_err_t ca_track_manager_unregister(ca_track *);

#ifdef __cplusplus
}
#endif

#endif /* __ESP32_RFID_LAP_COUNTER_TRACK_MANAGER_H__ */