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

#include "cmd.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static EventGroupHandle_t s_mqtt_event_group;

/* The event group allows multiple bits for each event, but we only care about one event */
/* - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;
const int WIFI_FAIL_BIT = BIT1;

/* - are we connected to the MQTT Server? */
const int MQTT_CONNECTED_BIT = BIT2;

static const char *TAG = "MAIN";

static int s_retry_num = 0;

QueueHandle_t xQueueCmd;

#define BOARD_ESP32CAM_AITHINKER

// WROVER-KIT PIN Map
#ifdef BOARD_WROVER_KIT

#define CAM_PIN_PWDN -1  //power down is not used
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 21
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 19
#define CAM_PIN_D2 18
#define CAM_PIN_D1 5
#define CAM_PIN_D0 4
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

#endif

// ESP32Cam (AiThinker) PIN Map
#ifdef BOARD_ESP32CAM_AITHINKER

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

#endif

//static camera_config_t camera_config = {
camera_config_t camera_config = {
	.pin_pwdn = CAM_PIN_PWDN,
	.pin_reset = CAM_PIN_RESET,
	.pin_xclk = CAM_PIN_XCLK,
	.pin_sscb_sda = CAM_PIN_SIOD,
	.pin_sscb_scl = CAM_PIN_SIOC,

	.pin_d7 = CAM_PIN_D7,
	.pin_d6 = CAM_PIN_D6,
	.pin_d5 = CAM_PIN_D5,
	.pin_d4 = CAM_PIN_D4,
	.pin_d3 = CAM_PIN_D3,
	.pin_d2 = CAM_PIN_D2,
	.pin_d1 = CAM_PIN_D1,
	.pin_d0 = CAM_PIN_D0,
	.pin_vsync = CAM_PIN_VSYNC,
	.pin_href = CAM_PIN_HREF,
	.pin_pclk = CAM_PIN_PCLK,

	//XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
	.xclk_freq_hz = 20000000,
	.ledc_timer = LEDC_TIMER_0,
	.ledc_channel = LEDC_CHANNEL_0,

	.pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
	.frame_size = FRAMESIZE_VGA,	//QQVGA-UXGA Do not use sizes above QVGA when not JPEG

	.jpeg_quality = 12, //0-63 lower number means higher quality
	.fb_count = 1		//if more than one, i2s runs in continuous mode. Use only with JPEG
};

static esp_err_t init_camera(int framesize)
{
	//initialize the camera
	camera_config.frame_size = framesize;
	esp_err_t err = esp_camera_init(&camera_config);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Camera Init Failed");
		return err;
	}

	return ESP_OK;
}

static void event_handler(void* arg, esp_event_base_t event_base,
								int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	// your_context_t *context = event->context;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED");
			xEventGroupClearBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
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

esp_err_t wifi_init_sta(void)
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&event_handler,
														NULL,
														&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														IP_EVENT_STA_GOT_IP,
														&event_handler,
														NULL,
														&instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_ESP_WIFI_SSID,
			.password = CONFIG_ESP_WIFI_PASSWORD,
			/* Setting a password implies station will connect to all security modes including WEP/WPA.
			 * However these modes are deprecated and not advisable to be used. Incase your Access point
			 * doesn't support WPA2, these mode can be enabled by commenting below line */
		 .threshold.authmode = WIFI_AUTH_WPA2_PSK,

			.pmf_cfg = {
				.capable = true,
				.required = false
			},
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	ESP_LOGI(TAG, "wifi_init_sta finished.");

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
			WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
			pdFALSE,
			pdFALSE,
			portMAX_DELAY);

	esp_err_t ret;
	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
				 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
		ret = ESP_OK;
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
				 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
		ret = ESP_FAIL;
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
		ret = ESP_FAIL;
	}

	/* The event will not be processed after unregister */
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	vEventGroupDelete(s_wifi_event_group);
	return ret;
}


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


void app_main(void)
{
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	if (wifi_init_sta() != ESP_OK) {
		ESP_LOGE(TAG, "wifi_init_sta fail");
		while(1) {
			vTaskDelay(1);
		}
	}


#if CONFIG_ENABLE_FLASH
	// Enable Flash Light
	gpio_pad_select_gpio(CONFIG_GPIO_FLASH);
	gpio_set_direction(CONFIG_GPIO_FLASH, GPIO_MODE_OUTPUT);
	gpio_set_level(CONFIG_GPIO_FLASH, 0);
#endif

	/* Create Queue */
	xQueueCmd = xQueueCreate( 10, sizeof(CMD_t) );
	configASSERT( xQueueCmd );

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

	s_mqtt_event_group = xEventGroupCreate();
	esp_mqtt_client_config_t mqtt_cfg = {
		.uri = CONFIG_BROKER_URL,
		.event_handle = mqtt_event_handler,
	};
	esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_start(mqtt_client);

#if CONFIG_FRAMESIZE_240X240
	int framesize = FRAMESIZE_240X240;
	#define	FRAMESIZE_STRING "240x240"
#elif CONFIG_FRAMESIZE_QVGA
	int framesize = FRAMESIZE_QVGA;
	#define	FRAMESIZE_STRING "320x240"
#elif CONFIG_FRAMESIZE_HVGA
	int framesize = FRAMESIZE_HVGA;
	#define	FRAMESIZE_STRING "480x320"
#elif CONFIG_FRAMESIZE_VGA
	int framesize = FRAMESIZE_VGA;
	#define	FRAMESIZE_STRING "640x480"
#elif CONFIG_FRAMESIZE_SVGA
	int framesize = FRAMESIZE_SVGA;
	#define	FRAMESIZE_STRING "800x600"
#elif CONFIG_FRAMESIZE_XGA
	int framesize = FRAMESIZE_XGA;
	#define	FRAMESIZE_STRING "1024x768"
#elif CONFIG_FRAMESIZE_HD
	int framesize = FRAMESIZE_HD;
	#define	FRAMESIZE_STRING "1280x720"
#elif CONFIG_FRAMESIZE_SXGA
	int framesize = FRAMESIZE_SXGA;
	#define	FRAMESIZE_STRING "1280x1024"
#elif CONFIG_FRAMESIZE_UXGA
	int framesize = FRAMESIZE_UXGA;
	#define	FRAMESIZE_STRING "1600x1200"
#endif

	init_camera(framesize);
	
	CMD_t cmdBuf;

	while (1) {
		ESP_LOGW(TAG,"Waitting %s ....", SHUTTER);
		xQueueReceive(xQueueCmd, &cmdBuf, portMAX_DELAY);
		ESP_LOGI(TAG, "cmdBuf.command=%d", cmdBuf.command);
		if (cmdBuf.command == CMD_HALT) break;

		EventBits_t mqttConnectBits = xEventGroupGetBits(s_mqtt_event_group);
		ESP_LOGI(TAG, "mqttConnectBits=%x", mqttConnectBits);
		if (mqttConnectBits & MQTT_CONNECTED_BIT) {

#if CONFIG_ENABLE_FLASH
			// Flash Light ON
			gpio_set_level(CONFIG_GPIO_FLASH, 1);
#endif

			// Capture frame
			camera_fb_t * fb = esp_camera_fb_get();
			if (fb) {
				ESP_LOGI(TAG, "Captured with %s", FRAMESIZE_STRING);
				//int msg_id = esp_mqtt_client_publish(mqtt_client, CONFIG_PUB_TOPIC, "test", 0, 1, 0);
				int msg_id = esp_mqtt_client_publish(mqtt_client, CONFIG_PUB_TOPIC, (char *)fb->buf, fb->len, 1, 0);
				if (msg_id < 0) {
					ESP_LOGE(TAG, "esp_mqtt_client_publish fail");
				} else {
					ESP_LOGI(TAG, "sent publish successful, msg_id=%d fb->len=%d", msg_id, fb->len);
				}

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
