#include <esp_log.h>
#include <esp_event.h>
#include <esp_timer.h>
#include <driver/gpio.h>

#include "ca_button.h"

ESP_EVENT_DEFINE_BASE(CA_BUTTON_EVENT);

static const char *TAG = "button";
static bool old_state = false;

static void IRAM_ATTR isr_handler(void *arg);

void ca_button_setup(void) {
    gpio_pad_select_gpio(GPIO_NUM_5);
    ESP_ERROR_CHECK(gpio_set_pull_mode(GPIO_NUM_5, GPIO_PULLUP_ONLY));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_intr_type(GPIO_NUM_5, GPIO_INTR_ANYEDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_NUM_5, isr_handler, NULL));
}

static void IRAM_ATTR isr_handler(void *arg) {
    bool new_state = !gpio_get_level(GPIO_NUM_5);
    BaseType_t task_woken = pdFALSE;

    if (new_state == true && old_state == false) {
        old_state = new_state;
        ESP_EARLY_LOGI(TAG, "Emitting PRESSED");
        ESP_ERROR_CHECK(esp_event_isr_post(CA_BUTTON_EVENT, CA_BUTTON_EVENT_PRESSED, NULL, 0, &task_woken));
    } else if (new_state == false && old_state == true) {
        old_state = new_state;
        ESP_EARLY_LOGI(TAG, "Emitting RELEASED");
        ESP_ERROR_CHECK(esp_event_isr_post(CA_BUTTON_EVENT, CA_BUTTON_EVENT_RELEASED, NULL, 0, &task_woken));
    }

    if (task_woken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}