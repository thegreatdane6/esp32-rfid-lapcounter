#include <esp_log.h>
#include <esp_event.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>

#include "ca_buzzer.h"

ESP_EVENT_DEFINE_BASE(CA_BUZZER_EVENT);

#define BUZZER_DEFAULT_DELAY (void *)(200 / portTICK_PERIOD_MS)

static const char *TAG = "buzzer";
static esp_timer_handle_t beep_timer;
static bool timer_running = false;

/* event handler */
static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);
static void timer_handler(void *arg);

void ca_buzzer_setup(void)
{
    const esp_timer_create_args_t timer_args = {
        .callback = timer_handler,
        .name = "Penis",
        .arg = BUZZER_DEFAULT_DELAY,
        .dispatch_method = ESP_TIMER_TASK,
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &beep_timer));

    gpio_pad_select_gpio(GPIO_NUM_17);
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_NUM_17, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_NUM_17, GPIO_INTR_DISABLE));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_BUZZER_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL, NULL));
}

static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data)
{
    switch (event_id)
    {
    case CA_BUZZER_EVENT_PERIODIC_BEEP:
        if (!timer_running)
        {
            ESP_ERROR_CHECK(esp_timer_start_periodic(beep_timer, 1000 / portTICK_PERIOD_MS));
            timer_running = true;
        }
        else
        {
            ESP_ERROR_CHECK(esp_timer_stop(beep_timer));
            timer_running = false;
        }
        break;
    case CA_BUZZER_EVENT_BEEP:
        if (event_data != NULL)
        {
            timer_handler((void *)*(uint32_t *)event_data);
        }
        else
        {
            timer_handler(BUZZER_DEFAULT_DELAY);
        }
        break;
    }
}

static void timer_handler(void *arg)
{
    gpio_set_level(GPIO_NUM_17, 1);
    vTaskDelay((TickType_t)arg);
    gpio_set_level(GPIO_NUM_17, 0);
}
