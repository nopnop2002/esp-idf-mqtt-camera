/* The example of MQTT Subscribe
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"

#include "cmd.h"
#include "mqtt.h"

#if CONFIG_SHUTTER_MQTT

static const char *TAG = "SUB";

extern EventGroupHandle_t status_event_group;
extern int WIFI_CONNECTED_BIT;
extern int MQTT_CONNECTED_BIT;

extern QueueHandle_t xQueueCmd;
extern QueueHandle_t xQueueSubscribe;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	// your_context_t *context = event->context;
	esp_mqtt_client_handle_t mqtt_client = event->client;

	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(status_event_group, MQTT_CONNECTED_BIT);
			esp_mqtt_client_subscribe(mqtt_client, CONFIG_SUB_TOPIC, 0);
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
			MQTT_t mqttBuf;
			//ESP_LOGI(TAG, "TOPIC=%.*s\r", event->topic_len, event->topic);
			//ESP_LOGI(TAG, "DATA=%.*s\r", event->data_len, event->data);
			mqttBuf.topic_type = SUBSCRIBE;
			mqttBuf.topic_len = event->topic_len;
			for(int i=0;i<event->topic_len;i++) {
				mqttBuf.topic[i] = event->topic[i];
				mqttBuf.topic[i+1] = 0;
			}
			mqttBuf.data_len = event->data_len;
			for(int i=0;i<event->data_len;i++) {
				mqttBuf.data[i] = event->data[i];
				mqttBuf.data[i+1] = 0;
			}
			xQueueSend(xQueueSubscribe, &mqttBuf, 0);
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

void mqtt_sub(void *pvParameters)
{
	ESP_LOGI(TAG, "Start Subscribe Broker:%s", CONFIG_BROKER_URL);
	CMD_t cmdBuf;
	cmdBuf.taskHandle = xTaskGetCurrentTaskHandle();
	cmdBuf.command = CMD_TAKE;

	esp_mqtt_client_config_t mqtt_cfg = {
		.uri = CONFIG_BROKER_URL,
		.event_handle = mqtt_event_handler,
		.client_id = "subscribe",
	};

	esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	xEventGroupClearBits(status_event_group, MQTT_CONNECTED_BIT);
	esp_mqtt_client_start(mqtt_client);
	xEventGroupWaitBits(status_event_group, MQTT_CONNECTED_BIT, false, true, portMAX_DELAY);
	ESP_LOGI(TAG, "Connect to MQTT Server");

	//esp_mqtt_client_subscribe(mqtt_client, CONFIG_SUB_TOPIC, 0);
	MQTT_t mqttBuf;
	while (1) {
		xQueueReceive(xQueueSubscribe, &mqttBuf, portMAX_DELAY);
		ESP_LOGI(TAG, "type=%d", mqttBuf.topic_type);

		if (mqttBuf.topic_type == SUBSCRIBE) {
			ESP_LOGI(TAG, "TOPIC=%.*s\r", mqttBuf.topic_len, mqttBuf.topic);
			ESP_LOGI(TAG, "DATA=%.*s\r", mqttBuf.data_len, mqttBuf.data);
			if (xQueueSend(xQueueCmd, &cmdBuf, 10) != pdPASS) {
				ESP_LOGE(TAG, "xQueueSend fail");
			}
		} // end if
	} // end while

	// Never reach here
	ESP_LOGI(TAG, "Task Delete");
	esp_mqtt_client_stop(mqtt_client);
	vTaskDelete(NULL);
}
#endif
