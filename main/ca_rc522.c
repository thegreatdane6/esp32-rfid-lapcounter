#include <esp_log.h>
#include <esp_event.h>

#include <rc522.h>

#include "ca_rc522.h"

spi_device_handle_t rc522_spi;
spi_device_handle_t rc522_spi2;
esp_timer_handle_t rc522_timer;
esp_timer_handle_t rc522_timer2;

ESP_EVENT_DEFINE_BASE(CA_RC522_EVENT);

/* logging tag */
static const char *TAG = "rc522";

static void tag_handler(spi_device_handle_t handle, uint8_t *serial_no);

void ca_rc522_setup(void)
{
    spi_bus_config_t buscfg = {
        .miso_io_num = 19,
        .mosi_io_num = 23,
        .sclk_io_num = 18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1};
    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &buscfg, 0));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 5000000,
        .mode = 0,
        .spics_io_num = 22,
        .queue_size = 7,
        .flags = SPI_DEVICE_HALFDUPLEX};
    ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &devcfg, &rc522_spi));
    ESP_ERROR_CHECK(rc522_init(rc522_spi));

    /* spi_device_interface_config_t devcfg2 = {
        .clock_speed_hz = 5000000,
        .mode = 0,
        .spics_io_num = 21,
        .queue_size = 7,
        .flags = SPI_DEVICE_HALFDUPLEX};

    ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &devcfg2, &rc522_spi2));
    ESP_ERROR_CHECK(rc522_init(rc522_spi2));*/

    rc522_tag_callback_data_t *arg = malloc(sizeof(rc522_tag_callback_data_t));
    arg->cb = tag_handler;
    arg->handle = rc522_spi;

    const esp_timer_create_args_t timer_args = {
        .callback = &rc522_timer_callback,
        .arg = (void *)arg};
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &rc522_timer));

    /*rc522_tag_callback_data_t *arg1 = malloc(sizeof(rc522_tag_callback_data_t));
    arg1->cb = tag_handler;
    arg1->handle = rc522_spi2;

    const esp_timer_create_args_t timer_args2 = {
        .callback = &rc522_timer_callback,
        .arg = (void *)arg1};
    ESP_ERROR_CHECK(esp_timer_create(&timer_args2, &rc522_timer2));*/

    ESP_ERROR_CHECK(esp_timer_start_periodic(rc522_timer, 125000));
    //ESP_ERROR_CHECK(esp_timer_start_periodic(rc522_timer2, 125000));
}

static void tag_handler(spi_device_handle_t handle, uint8_t *serial_no)
{
    ESP_ERROR_CHECK(esp_event_post(CA_RC522_EVENT, CA_RC522_EVENT_TRIGGER, serial_no, 5, portMAX_DELAY));
}