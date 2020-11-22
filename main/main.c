/* 
   This code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "mqtt_client.h"

#include "esp_camera.h"
#include "camera.h"
#include "cmd.h"

/* FreeRTOS event group to signal when we are connected*/
EventGroupHandle_t status_event_group;

/* The event group allows multiple bits for each event, but we only care about one event */
/* - are we connected to the AP with an IP? */
int WIFI_CONNECTED_BIT = BIT2;
/* - are we connected to the MQTT Server? */
int MQTT_CONNECTED_BIT = BIT4;

QueueHandle_t xQueueCmd;

static const char *TAG = "MAIN";

static int s_retry_num = 0;

void mqtt_pub(void *pvParameters);

#if CONFIG_SHUTTER_ENTER
void keyin(void *pvParameters);
#endif

#if CONFIG_SHUTTER_GPIO
void gpio(void *pvParameters);
#endif

#if CONFIG_SHUTTER_MQTT
void mqtt_sub(void *pvParameters);
#endif

static void event_handler(void* arg, esp_event_base_t event_base,
								int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
			esp_wifi_connect();
			xEventGroupClearBits(status_event_group, WIFI_CONNECTED_BIT);
			s_retry_num++;
			ESP_LOGW(TAG, "retry to connect to the AP");
		} else {
			ESP_LOGE(TAG,"connect to the AP fail");
			while(1) {
				vTaskDelay(1000/portTICK_PERIOD_MS);
			}
		}
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(status_event_group, WIFI_CONNECTED_BIT);
	}
}



static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	// your_context_t *context = event->context;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(status_event_group, MQTT_CONNECTED_BIT);
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED");
			xEventGroupClearBits(status_event_group, MQTT_CONNECTED_BIT);
			break;
		case MQTT_EVENT_SUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_UNSUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_PUBLISHED:
			ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_DATA:
			ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			break;
		case MQTT_EVENT_ERROR:
			ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
			break;
		default:
			ESP_LOGI(TAG, "Other event id:%d", event->event_id);
			break;
	}
	return ESP_OK;
}


void wifi_init_sta(void)
{
	status_event_group = xEventGroupCreate();

#if 0
	tcpip_adapter_init();

	ESP_ERROR_CHECK(esp_event_loop_create_default());
#endif
	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_ESP_WIFI_SSID,
			.password = CONFIG_ESP_WIFI_PASSWORD
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	xEventGroupWaitBits(status_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s", CONFIG_ESP_WIFI_SSID);
}


void app_main(void)
{
	ESP_LOGI(TAG, "Startup..");
	ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	wifi_init_sta();


#if CONFIG_ENABLE_FLASH
	// Enable Flash Light
	gpio_pad_select_gpio(CONFIG_GPIO_FLASH);
	gpio_set_direction(CONFIG_GPIO_FLASH, GPIO_MODE_OUTPUT);
	gpio_set_level(CONFIG_GPIO_FLASH, 0);
#endif

	/* Create Queue */
	xQueueCmd = xQueueCreate( 10, sizeof(CMD_t) );
	configASSERT( xQueueCmd );

#if 0
	/* Start task */
	xTaskCreate(mqtt_pub, "PUB", 1024*4, NULL, 2, NULL);
#endif

	/* Create Shutter Task */
#if CONFIG_SHUTTER_ENTER
#define SHUTTER "Keybord Enter"
	xTaskCreate(keyin, "KEYIN", 1024*4, NULL, 2, NULL);
#endif

#if CONFIG_SHUTTER_GPIO
#define SHUTTER "GPIO Input"
	xTaskCreate(gpio, "GPIO", 1024*4, NULL, 2, NULL);
#endif

#if CONFIG_SHUTTER_MQTT
#define SHUTTER "MQTT Input"
	xTaskCreate(mqtt_sub, "SUB", 1024*4, NULL, 2, NULL);
#endif

	esp_mqtt_client_config_t mqtt_cfg = {
		.uri = CONFIG_BROKER_URL,
		.event_handle = mqtt_event_handler,
		.client_id = "publish",
	};
	esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_start(mqtt_client);

	/* Detect camera */
	ret = camera_detect();
	if (ret != ESP_OK) return;
	
    CMD_t cmdBuf;
    while (1) {
		ESP_LOGI(TAG,"Waitting %s ....", SHUTTER);
        xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
        ESP_LOGI(TAG, "cmdBuf.command=%d", cmdBuf.command);
        if (cmdBuf.command == CMD_HALT) break;

        EventBits_t EventBits = xEventGroupGetBits(status_event_group);
        ESP_LOGI(TAG, "EventBits=%x", EventBits);
        if (EventBits & MQTT_CONNECTED_BIT) {

#if CONFIG_ENABLE_FLASH
            // Flash Light ON
            gpio_set_level(CONFIG_GPIO_FLASH, 1);
#endif

            //acquire a frame
            camera_fb_t * fb = esp_camera_fb_get();
            if (fb) {
                //int msg_id = esp_mqtt_client_publish(mqtt_client, CONFIG_PUB_TOPIC, "test", 0, 1, 0);
                int msg_id = esp_mqtt_client_publish(mqtt_client, CONFIG_PUB_TOPIC, (char *)fb->buf, fb->len, 1, 0);
                ESP_LOGI(TAG, "sent publish successful, msg_id=%d fb->len=%d", msg_id, fb->len);

                //return the frame buffer back to the driver for reuse
                esp_camera_fb_return(fb);
            } else {
                ESP_LOGE(TAG, "Camera Capture Failed");
            }


#if CONFIG_ENABLE_FLASH
            // Flash Light OFF
            gpio_set_level(CONFIG_GPIO_FLASH, 0);
#endif
        } else {
            ESP_LOGW(TAG, "MQTT server not connected");
        }
	} // end while
}
