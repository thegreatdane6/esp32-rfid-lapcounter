#include <esp_log.h>
#include <esp_event.h>

#include "ca_ledstrip.h"
#include "ca_race_director.h"
#include "ca_buzzer.h"

ESP_EVENT_DEFINE_BASE(CA_RACE_DIRECTOR_EVENT);

/* logging tag */
static const char *TAG = "race_director";

/* event handler */
static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);

void ca_race_director_setup(void)
{
    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_RACE_DIRECTOR_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_LEDSTRIP_EVENT, CA_LEDSTRIP_EVENT_ANIMATE_FINISH, event_handler, NULL, NULL));
}

static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data)
{
    if (event_base == CA_RACE_DIRECTOR_EVENT)
    {
        switch (event_id)
        {
        case CA_RACE_DIRECTOR_EVENT_START_RACE:
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
            break;
        }
        }
    }
    else if (event_base == CA_LEDSTRIP_EVENT && event_id == CA_LEDSTRIP_EVENT_ANIMATE_FINISH)
    {
        ESP_ERROR_CHECK(esp_event_post(CA_BUZZER_EVENT, CA_BUZZER_EVENT_BEEP, NULL, 0, portMAX_DELAY));
    }
}