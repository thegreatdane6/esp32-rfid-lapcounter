#include <esp_log.h>
#include <esp_event.h>
#include <esp_ota_ops.h>
#include <esp_http_client.h>
#include <esp_flash_partitions.h>
#include <esp_partition.h>

#include "ca_ota.h"

/* SHA-256 digest length */
#define HASH_LEN 32

/* defines CA_OTA_EVENT used by ESP event loop */
ESP_EVENT_DEFINE_BASE(CA_OTA_EVENT);

/* logging tag */
static const char *TAG = "ota";

/* ota timer handle */
static esp_timer_handle_t timer_handle;

/* semaphore used to block */
static SemaphoreHandle_t semaphore = NULL;

/* http event handler */
static esp_err_t http_event_handler(esp_http_client_event_t *evt);

/* event handler */
static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data);

/* timer event */
static void timer_check(void *arg);

/* prints sha-256 */
static void print_sha256(const uint8_t *image_hash, const char *label);

/* prints partition info */
static void print_partition_info(const esp_partition_t *running);

void ca_ota_setup(void)
{
    ESP_LOGD(TAG, "create semaphore mutext");
    semaphore = xSemaphoreCreateMutex();
    if (semaphore == NULL)
    {
        ESP_LOGE(TAG, "unable to create semaphore mutext");
        abort();
    }

    ESP_LOGD(TAG, "get partition info of currently configured boot app");
    const esp_partition_t *configured = esp_ota_get_boot_partition();

    ESP_LOGD(TAG, "get partition info of currently running app");
    const esp_partition_t *running = esp_ota_get_running_partition();

    ESP_LOGD(TAG, "get partition info for the update");
    const esp_partition_t *update = esp_ota_get_next_update_partition(running);

#if CONFIG_LOG_DEFAULT_LEVEL >= 2
    ESP_LOGD(TAG, "print running partition info");
    print_partition_info(running);
#endif /* CONFIG_BOOTLOADER_LOG_LEVEL */

    if (configured != running)
    {
        ESP_LOGW(TAG, "configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
        ESP_LOGW(TAG, "(this can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }

    ESP_LOGI(TAG, "running partition type %d subtype %d at offset 0x%08x", running->type, running->subtype, running->address);

    assert(update != NULL);
    ESP_LOGI(TAG, "update partition type %d subtype %d at offset 0x%08x", update->type, update->subtype, update->address);

    const esp_timer_create_args_t timer_args = {
        .name = "ota",
        .dispatch_method = ESP_TIMER_TASK,
        .callback = timer_check,
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer_handle));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(CA_OTA_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
}

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_HEADER:
        break;
    case HTTP_EVENT_ON_DATA:
        break;
    default:
        break;
    }

    // TODO: check if available
    return ESP_OK;
}

static void event_handler(void *arg, esp_event_base_t event_base, int event_id, void *event_data)
{
    if (event_base == CA_OTA_EVENT)
    {
        switch (event_id)
        {
        case CA_OTA_EVENT_CHECK:
        {
            esp_http_client_handle_t client;
            esp_err_t err;
           
            if (event_data == NULL)
            {
                ESP_LOGD(TAG, "configure HTTP Client");
                esp_http_client_config_t config = {
                    .url = "https://192.168.178.49:1443/api/v1/check",
                    .is_async = true,
                    // .use_global_ca_store = true,
                    .skip_cert_common_name_check = true,
                    .event_handler = http_event_handler,
                };

                ESP_LOGD(TAG, "initialize HTTP client");
                client = esp_http_client_init(&config);

                ESP_LOGD(TAG, "checking for OTA update");
            } else {
                client = *(esp_http_client_handle_t *)event_data;
            }

            if ((err = esp_http_client_perform(client)) == ESP_ERR_HTTP_EAGAIN)
            {
                ESP_ERROR_CHECK(esp_event_post(CA_OTA_EVENT, CA_OTA_EVENT_CHECK, (void *)&client, sizeof(esp_http_client_handle_t *), portMAX_DELAY));
                break;
            }

            if (err != ESP_OK)
            {
                ESP_ERROR_CHECK(esp_event_post(CA_OTA_EVENT, CA_OTA_EVENT_FAILED, (void *)&err, sizeof(err), portMAX_DELAY));
                ESP_ERROR_CHECK(esp_event_post(CA_OTA_EVENT, CA_OTA_EVENT_CLEANUP, (void *)&client, sizeof(esp_http_client_handle_t *), portMAX_DELAY));
                break;
            }

            // TODO: We should have data here already
            break;
        }
        case CA_OTA_EVENT_FAILED:
        {
            ESP_LOGE(TAG, "OTA update failed %s", esp_err_to_name((esp_err_t)event_data));
            break;
        }
        case CA_OTA_EVENT_CLEANUP:
        {
            ESP_ERROR_CHECK(esp_http_client_cleanup(*(esp_http_client_handle_t *)event_data));
            xSemaphoreGive(semaphore);
            break;
        }
        default:
            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        switch (event_id)
        {
        case IP_EVENT_ETH_GOT_IP:
        case IP_EVENT_PPP_GOT_IP:
        case IP_EVENT_STA_GOT_IP:
            ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, 15 * 60 * 1000 * 1000)); // every 15 min

            if (xSemaphoreTake(semaphore, (TickType_t)10) == pdTRUE)
                ESP_ERROR_CHECK(esp_event_post(CA_OTA_EVENT, CA_OTA_EVENT_CHECK, NULL, 0, portMAX_DELAY));
            break;

        case IP_EVENT_PPP_LOST_IP:
        case IP_EVENT_STA_LOST_IP:
            ESP_ERROR_CHECK(esp_timer_stop(timer_handle));
            xSemaphoreGive(semaphore);
            break;
        }
    }
}

static void timer_check(void *arg)
{
    if (xSemaphoreTake(semaphore, (TickType_t)10) == pdTRUE)
        ESP_ERROR_CHECK(esp_event_post(CA_OTA_EVENT, CA_OTA_EVENT_CHECK, NULL, 0, portMAX_DELAY));
}

static void print_partition_info(const esp_partition_t *running)
{
    uint8_t sha_256[HASH_LEN] = {0};
    esp_partition_t partition;

    /* get sha256 digest for the partition table */
    partition.address = ESP_PARTITION_TABLE_OFFSET;
    partition.size = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for the partition table: ");

    /* get sha256 digest for bootloader */
    partition.address = ESP_BOOTLOADER_OFFSET;
    partition.size = ESP_PARTITION_TABLE_OFFSET;
    partition.type = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    /* get sha256 digest for running partition */
    esp_partition_get_sha256(running, sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");
}

static void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i)
    {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}