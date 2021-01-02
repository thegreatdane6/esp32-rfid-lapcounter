#include <esp_log.h>
#include <esp_event.h>

#include <memory.h>

#include "ca_ledstrip.h"
#include "ca_race_director.h"
#include "ca_buzzer.h"
#include "ca_rc522.h"

ESP_EVENT_DEFINE_BASE(CA_RACE_DIRECTOR_EVENT);

typedef struct lap_t lap_t;
typedef struct driver_t driver_t;

struct lap_t
{
    lap_t *next;
    uint32_t time;
};

struct driver_t
{
    uint8_t chip_id[32];
    const char *name;
    lap_t *laps;
    driver_t *next;
};

typedef struct
{
    bool started;
    driver_t *drivers;
} race_state_t;

/* logging tag */
static const char *TAG = "race_director";

static esp_timer_handle_t race_timer;

static void timer_handler(void *arg);
/* event handler */
static void led_event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);
static void race_event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);
static void rc522_event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);

static race_state_t RACE_STATE = {0};

void ca_race_director_setup(void)
{
    const esp_timer_create_args_t timer_args = {
        .callback = timer_handler,
        .name = "Penis",
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &race_timer));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_RACE_DIRECTOR_EVENT, ESP_EVENT_ANY_ID, race_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_LEDSTRIP_EVENT, ESP_EVENT_ANY_ID, led_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_RC522_EVENT, ESP_EVENT_ANY_ID, rc522_event_handler, NULL, NULL));
}

static void race_event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data)
{
    switch (event_id)
    {
    case CA_RACE_DIRECTOR_EVENT_START_RACE:
    {
        if (!RACE_STATE.started)
        {
            uint32_t size = 6;
            ca_ledstrip_animation_step_t *steps = calloc(size, sizeof(ca_ledstrip_animation_step_t));
            for (int i = 0; i < size; i++)
            {
                steps[i].id = i * 3;
                steps[i].color = 0xFF0000;
                steps[i].delay = 750 / portTICK_RATE_MS;
            }
            steps[5].delay = 0;
            steps[5].anim_type = CA_LEDSTRIP_ANIMATION_RESET;

            ca_ledstrip_animation_t animation = {
                .steps = steps,
                .length = size,
            };
            ESP_ERROR_CHECK(esp_event_post(CA_LEDSTRIP_EVENT, CA_LEDSTRIP_EVENT_ANIMATE, &animation, sizeof(ca_ledstrip_animation_t), portMAX_DELAY));
            RACE_STATE.started = true;
        }
        break;
    }
    case CA_RACE_DIRECTOR_EVENT_END_RACE:
    {
        if (RACE_STATE.started)
        {
            uint32_t value = (1000 / portTICK_PERIOD_MS);
            ESP_ERROR_CHECK(esp_event_post(CA_BUZZER_EVENT, CA_BUZZER_EVENT_BEEP, &value, sizeof(value), portMAX_DELAY));
            RACE_STATE.started = false;
        }
    }
    }
}

static void led_event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data)
{
    if (event_id == CA_LEDSTRIP_EVENT_ANIMATE_FINISH && RACE_STATE.started == true)
    {
        ESP_ERROR_CHECK(esp_event_post(CA_BUZZER_EVENT, CA_BUZZER_EVENT_BEEP, NULL, 0, portMAX_DELAY));
        ESP_ERROR_CHECK(esp_timer_start_once(race_timer, 5 * 1000000));
    }
}

static void timer_handler(void *arg)
{
    ESP_ERROR_CHECK(esp_event_post(CA_RACE_DIRECTOR_EVENT, CA_RACE_DIRECTOR_EVENT_END_RACE, NULL, 0, portMAX_DELAY));
}

static void rc522_event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data)
{
    switch (event_id)
    {
    case CA_RC522_EVENT_TRIGGER:
    {
        uint8_t *data = (uint8_t *)event_data;
        uint32_t timestamp = esp_log_timestamp();

        driver_t *driver = RACE_STATE.drivers;
        if (driver == NULL)
        {
        register_driver:
            driver = RACE_STATE.drivers = calloc(1, sizeof(driver_t));
            memcpy(driver->chip_id, data, 5);
            driver->name = "Dane";
            ESP_LOGI(TAG, "Register driver %s", driver->chip_id);
        }

        do
        {
            if (memcmp(driver->chip_id, data, 5) != 0)
            {
                driver = driver->next;
                continue;
            }

            if (driver->laps && timestamp - driver->laps->time < 3000)
            {
                goto done;
            }

            lap_t *lap = calloc(1, sizeof(lap_t));
            lap->time = timestamp;
            lap->next = driver->laps;
            driver->laps = lap;
            ESP_LOGI(TAG, "Lap driver %s", driver->chip_id);
            ESP_ERROR_CHECK(esp_event_post(CA_BUZZER_EVENT, CA_BUZZER_EVENT_BEEP, NULL, NULL, portMAX_DELAY));
            goto done;

        } while (driver != NULL);
        goto register_driver;
    done:
        break;
    }
    }
}