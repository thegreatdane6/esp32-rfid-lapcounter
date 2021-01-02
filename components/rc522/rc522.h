#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/spi_master.h"

typedef void (*rc522_tag_callback_t)(spi_device_handle_t handle, uint8_t *);

typedef struct {
    rc522_tag_callback_t cb;
    spi_device_handle_t handle;
} rc522_tag_callback_data_t;

esp_err_t rc522_write_n(spi_device_handle_t handle, uint8_t addr, uint8_t n, uint8_t *data);
esp_err_t rc522_write(spi_device_handle_t handle, uint8_t addr, uint8_t val);
uint8_t *rc522_read_n(spi_device_handle_t handle, uint8_t addr, uint8_t n);
uint8_t rc522_read(spi_device_handle_t handle, uint8_t addr);
#define rc522_fw_version(handle) rc522_read(handle, 0x37)
esp_err_t rc522_init(spi_device_handle_t handle);

esp_err_t rc522_set_bitmask(spi_device_handle_t handle, uint8_t addr, uint8_t mask);
esp_err_t rc522_clear_bitmask(spi_device_handle_t handle, uint8_t addr, uint8_t mask);
esp_err_t rc522_antenna_on(spi_device_handle_t handle);
uint8_t *rc522_calculate_crc(spi_device_handle_t handle, uint8_t *data, uint8_t n);
uint8_t *rc522_card_write(spi_device_handle_t handle, uint8_t cmd, uint8_t *data, uint8_t n, uint8_t *res_n);
uint8_t *rc522_request(spi_device_handle_t handle, uint8_t *res_n);
uint8_t *rc522_anticoll(spi_device_handle_t handle);
uint8_t *rc522_get_tag(spi_device_handle_t handle);

void rc522_timer_callback(void *arg);

#ifdef __cplusplus
}
#endif