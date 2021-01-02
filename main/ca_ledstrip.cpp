#include <esp_log.h>
#include <esp_event.h>
#include <FastLED.h>

#include "ca_ledstrip.h"

#define DATA_PIN 16
#define NUM_LEDS 13

ESP_EVENT_DEFINE_BASE(CA_LEDSTRIP_EVENT);

static const char *TAG = "ledstrip";

// Define the array of leds
static CRGB leds[NUM_LEDS];

/* event handler */
static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);

static void reset_strip(bool apply);

void ca_ledstrip_setup(void)
{
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(LEDColorCorrection::TypicalSMD5050);
    FastLED.setBrightness(255);

    reset_strip(true);

    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_LEDSTRIP_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL, NULL));
}

static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data)
{
    switch (event_id)
    {
    case CA_LEDSTRIP_EVENT_RESET:
    {
        reset_strip(true);
        break;
    }

    case CA_LEDSTRIP_EVENT_ANIMATE:
    {
        ca_ledstrip_animation_t *anim = (ca_ledstrip_animation_t *)event_data;

        ESP_ERROR_CHECK(esp_event_post(CA_LEDSTRIP_EVENT, CA_LEDSTRIP_EVENT_ANIMATE_START, NULL, 0, portMAX_DELAY));

        for (int i = 0; i < anim->length; i++)
        {
            ca_ledstrip_animation_step_t *step = &anim->steps[i];
            if (step->anim_type == CA_LEDSTRIP_ANIMATION_RESET)
            {
                reset_strip(false);
            }
            else
            {
                leds[step->id] = step->color;
                if (step->span > 1)
                {
                    for (int i = 1; i < step->span; i++)
                        leds[step->id + i] = step->color;
                }
            }

            FastLED.show();

            if (step->delay > 0)
                vTaskDelay(step->delay);
        }

        free(anim->steps);

        ESP_ERROR_CHECK(esp_event_post(CA_LEDSTRIP_EVENT, CA_LEDSTRIP_EVENT_ANIMATE_FINISH, NULL, 0, portMAX_DELAY));

        break;
    }
    }
}

static void reset_strip(bool apply)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB::Black;
    }

    if (apply)
        FastLED.show();
}