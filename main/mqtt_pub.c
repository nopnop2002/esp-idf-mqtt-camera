/* The example of MQTT Publish
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"

#include "esp_camera.h"
#include "camera.h"
#include "cmd.h"

static const char *TAG = "PUB";

extern EventGroupHandle_t status_event_group;
extern int WIFI_CONNECTED_BIT;
extern int MQTT_CONNECTED_BIT;

extern QueueHandle_t xQueueCmd;

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

void mqtt_pub(void *pvParameters)
{
	ESP_LOGI(TAG, "Start Publish Broker:%s", CONFIG_BROKER_URL);

	esp_mqtt_client_config_t mqtt_cfg = {
		.uri = CONFIG_BROKER_URL,
		.event_handle = mqtt_event_handler,
		.client_id = "publish",
	};

	esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	xEventGroupClearBits(status_event_group, MQTT_CONNECTED_BIT);
	esp_mqtt_client_start(mqtt_client);
	xEventGroupWaitBits(status_event_group, MQTT_CONNECTED_BIT, false, true, portMAX_DELAY);
	//ESP_LOGI(TAG, "Connect to MQTT Server");

	/* Detect camera */
	esp_err_t ret = camera_detect();
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Can't detect camera.");
		while(1);
	}

	CMD_t cmdBuf;
	while (1) {
		ESP_LOGI(TAG, "The shutter is ready");
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

#if 0
			if (!fb) {
				ESP_LOGE(TAG, "Camera Capture Failed");
				continue;
			}
#endif

#if CONFIG_ENABLE_FLASH
			// Flash Light OFF
			gpio_set_level(CONFIG_GPIO_FLASH, 0);
#endif
		} else {
			ESP_LOGW(TAG, "MQTT server not connected");
		}
	} // end while

	esp_mqtt_client_stop(mqtt_client);
	vTaskDelete(NULL);
}
