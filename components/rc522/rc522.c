#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "rc522.h"

static const char *TAG = "rc522";

esp_err_t rc522_write_n(spi_device_handle_t handle, uint8_t addr, uint8_t n, uint8_t *data) {
    uint8_t *buffer = (uint8_t *)malloc(n + 1);
    buffer[0] = (addr << 1) & 0x7E;

    for (uint8_t i = 1; i <= n; i++) {
        buffer[i] = data[i - 1];
    }

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    t.length = 8 * (n + 1);
    t.tx_buffer = buffer;

    esp_err_t ret = spi_device_transmit(handle, &t);

    free(buffer);

    return ret;
}

esp_err_t rc522_write(spi_device_handle_t handle, uint8_t addr, uint8_t val) {
    return rc522_write_n(handle, addr, 1, &val);
}

/* Returns pointer to dynamically allocated array of N places. */
uint8_t *rc522_read_n(spi_device_handle_t handle, uint8_t addr, uint8_t n) {
    if (n <= 0) {
        return NULL;
    }

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    uint8_t *buffer = (uint8_t *)malloc(n);

    t.flags = SPI_TRANS_USE_TXDATA;
    t.length = 8;
    t.tx_data[0] = ((addr << 1) & 0x7E) | 0x80;
    t.rxlength = 8 * n;
    t.rx_buffer = buffer;

    esp_err_t ret = spi_device_transmit(handle, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Invalid transmit: %s", esp_err_to_name(ret));
    }

    return buffer;
}

uint8_t rc522_read(spi_device_handle_t handle, uint8_t addr) {
    uint8_t *buffer = rc522_read_n(handle, addr, 1);
    uint8_t res = buffer[0];
    free(buffer);

    return res;
}

esp_err_t rc522_init(spi_device_handle_t handle) {
    // ---------- RW test ------------
    rc522_write(handle, 0x24, 0x25);
    if (rc522_read(handle, 0x24) != 0x25) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    rc522_write(handle, 0x24, 0x26);
    if (rc522_read(handle, 0x24) != 0x26) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    // ------- End of RW test --------

    rc522_write(handle, 0x01, 0x0F);
    rc522_write(handle, 0x2A, 0x8D);
    rc522_write(handle, 0x2B, 0x3E);
    rc522_write(handle, 0x2D, 0x1E);
    rc522_write(handle, 0x2C, 0x00);
    rc522_write(handle, 0x15, 0x40);
    rc522_write(handle, 0x11, 0x3D);

    rc522_antenna_on(handle);

    printf("RC522 Firmware 0x%x\n", rc522_fw_version(handle));

    return ESP_OK;
}

esp_err_t rc522_set_bitmask(spi_device_handle_t handle, uint8_t addr, uint8_t mask) {
    return rc522_write(handle, addr, rc522_read(handle, addr) | mask);
}

esp_err_t rc522_clear_bitmask(spi_device_handle_t handle, uint8_t addr, uint8_t mask) {
    return rc522_write(handle, addr, rc522_read(handle, addr) & ~mask);
}

esp_err_t rc522_antenna_on(spi_device_handle_t handle) {
    esp_err_t ret;

    if (~(rc522_read(handle, 0x14) & 0x03)) {
        ret = rc522_set_bitmask(handle, 0x14, 0x03);

        if (ret != ESP_OK) {
            return ret;
        }
    }

    //return rc522_write(0x26, 0x60); // 43dB gain
    return rc522_write(handle, 0x26, (0x07 << 4));
}

/* Returns pointer to dynamically allocated array of two element */
uint8_t *rc522_calculate_crc(spi_device_handle_t handle, uint8_t *data, uint8_t n) {
    rc522_clear_bitmask(handle, 0x05, 0x04);
    rc522_set_bitmask(handle, 0x0A, 0x80);

    rc522_write_n(handle, 0x09, n, data);

    rc522_write(handle, 0x01, 0x03);

    uint8_t i = 255;
    uint8_t nn = 0;

    for (;;) {
        nn = rc522_read(handle, 0x05);
        i--;

        if (!(i != 0 && !(nn & 0x04))) {
            break;
        }
    }

    uint8_t *res = (uint8_t *)malloc(2);

    res[0] = rc522_read(handle, 0x22);
    res[1] = rc522_read(handle, 0x21);

    return res;
}

uint8_t *rc522_card_write(spi_device_handle_t handle, uint8_t cmd, uint8_t *data, uint8_t n, uint8_t *res_n) {
    uint8_t *result = NULL;
    uint8_t irq = 0x00;
    uint8_t irq_wait = 0x00;
    uint8_t last_bits = 0;
    uint8_t nn = 0;

    if (cmd == 0x0E) {
        irq = 0x12;
        irq_wait = 0x10;
    } else if (cmd == 0x0C) {
        irq = 0x77;
        irq_wait = 0x30;
    }

    rc522_write(handle, 0x02, irq | 0x80);
    rc522_clear_bitmask(handle, 0x04, 0x80);
    rc522_set_bitmask(handle, 0x0A, 0x80);
    rc522_write(handle, 0x01, 0x00);

    rc522_write_n(handle, 0x09, n, data);

    rc522_write(handle, 0x01, cmd);

    if (cmd == 0x0C) {
        rc522_set_bitmask(handle, 0x0D, 0x80);
    }

    uint16_t i = 1000;

    for (;;) {
        nn = rc522_read(handle, 0x04);
        i--;

        if (!(i != 0 && (((nn & 0x01) == 0) && ((nn & irq_wait) == 0)))) {
            break;
        }
    }

    rc522_clear_bitmask(handle, 0x0D, 0x80);

    if (i != 0) {
        if ((rc522_read(handle, 0x06) & 0x1B) == 0x00) {
            if (cmd == 0x0C) {
                nn = rc522_read(handle, 0x0A);
                last_bits = rc522_read(handle, 0x0C) & 0x07;

                if (last_bits != 0) {
                    *res_n = (nn - 1) + last_bits;
                } else {
                    *res_n = nn;
                }

                if (*res_n != 0) {
                    result = (uint8_t *)malloc(*res_n);

                    for (i = 0; i < *res_n; i++) {
                        result[i] = rc522_read(handle, 0x09);
                    }
                }
            }
        }
    }

    return result;
}

uint8_t *rc522_request(spi_device_handle_t handle, uint8_t *res_n) {
    uint8_t *result = NULL;
    rc522_write(handle, 0x0D, 0x07);

    uint8_t req_mode = 0x26;
    result = rc522_card_write(handle, 0x0C, &req_mode, 1, res_n);

    if (*res_n * 8 != 0x10) {
        free(result);
        return NULL;
    }

    return result;
}

uint8_t *rc522_anticoll(spi_device_handle_t handle) {
    uint8_t *result = NULL;
    uint8_t res_n;
    uint8_t serial_number[] = {0x93, 0x20};

    rc522_write(handle, 0x0D, 0x00);

    result = rc522_card_write(handle, 0x0C, serial_number, 2, &res_n);

    if (result != NULL && res_n != 5) {
        free(result);
        return NULL;
    }

    return result;
}

uint8_t *rc522_get_tag(spi_device_handle_t handle) {
    uint8_t *result = NULL;
    uint8_t *res_data = NULL;
    uint8_t res_data_n;

    res_data = rc522_request(handle, &res_data_n);

    if (res_data != NULL) {
        free(res_data);

        result = rc522_anticoll(handle);

        if (result != NULL) {
            uint8_t buf[] = {0x50, 0x00, 0x00, 0x00};
            uint8_t *crc = rc522_calculate_crc(handle, buf, 2);

            buf[2] = crc[0];
            buf[3] = crc[1];

            free(crc);

            res_data = rc522_card_write(handle, 0x0C, buf, 4, &res_data_n);
            if (res_data != NULL) {
                free(res_data);
            }

            rc522_clear_bitmask(handle, 0x08, 0x08);

            return result;
        }
    }

    return NULL;
}

void rc522_timer_callback(void *arg) {
    rc522_tag_callback_data_t *data = (rc522_tag_callback_data_t *)arg;

    uint8_t *serial_no = rc522_get_tag(data->handle);

    if (serial_no != NULL) {
        data->cb(data->handle, serial_no);
        free(serial_no);
    }
}
