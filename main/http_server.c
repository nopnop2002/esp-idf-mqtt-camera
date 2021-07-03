/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <sys/stat.h>
#include <mbedtls/base64.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"

#include "http.h"

static const char *TAG = "HTTP";

extern QueueHandle_t xQueueHttp;

char * localFileName = NULL;

#if 0
static void SPIFFS_Directory(char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}
#endif

// Calculate the size after conversion to base64
// http://akabanessa.blog73.fc2.com/blog-entry-83.html
int32_t calcBase64EncodedSize(int origDataSize)
{
	// 6bit単位のブロック数（6bit単位で切り上げ）
	// Number of blocks in 6-bit units (rounded up in 6-bit units)
	int32_t numBlocks6 = ((origDataSize * 8) + 5) / 6;
	// 4文字単位のブロック数（4文字単位で切り上げ）
	// Number of blocks in units of 4 characters (rounded up in units of 4 characters)
	int32_t numBlocks4 = (numBlocks6 + 3) / 4;
	// 改行を含まない文字数
	// Number of characters without line breaks
	int32_t numNetChars = numBlocks4 * 4;
	// 76文字ごとの改行（改行は "\r\n" とする）を考慮したサイズ
	// Size considering line breaks every 76 characters (line breaks are "\ r \ n")
	//return numNetChars + ((numNetChars / 76) * 2);
	return numNetChars;
}

// Convert image to BASE64
esp_err_t Image2Base64(char * filename, size_t fsize, unsigned char * base64_buffer, size_t base64_buffer_len)
{
	unsigned char* image_buffer = NULL;
	//image_buffer = malloc(fsize + 1);
	image_buffer = malloc(fsize);
	if (image_buffer == NULL) {
		ESP_LOGE(TAG, "malloc fail. image_buffer %d", fsize);
		return ESP_FAIL;
	}

	FILE * fp;
	if((fp=fopen(filename,"rb"))==NULL){
		ESP_LOGE(TAG, "fopen fail. [%s]", filename);
		return ESP_FAIL;
	}else{
		for (int i=0;i<fsize;i++) {
			fread(&image_buffer[i],sizeof(char),1,fp);
		}
		fclose(fp);
	}

	size_t encord_len;
	esp_err_t ret = mbedtls_base64_encode(base64_buffer, base64_buffer_len, &encord_len, image_buffer, fsize);
	ESP_LOGI(TAG, "mbedtls_base64_encode=%d encord_len=%d", ret, encord_len);
	free(image_buffer);
	return ret;
}

/* An HTTP GET handler */
static esp_err_t root_get_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "root_get_handler");
	if (localFileName == NULL) {
		httpd_resp_sendstr_chunk(req, NULL);
		return ESP_OK;
	}

	struct stat st;
	if (stat(localFileName, &st) != 0) {
		ESP_LOGE(TAG, "[%s] not found", localFileName);
		httpd_resp_sendstr_chunk(req, NULL);
		return ESP_OK;
	}

	ESP_LOGI(TAG, "%s exist st.st_size=%ld", localFileName, st.st_size);
	int32_t base64Size = calcBase64EncodedSize(st.st_size);
	ESP_LOGI(TAG, "base64Size=%d", base64Size);

	/* Send HTML file header */
	httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><body>");

	// Convert from JPEG to BASE64
	unsigned char*	img_src_buffer = NULL;
 	size_t img_src_buffer_len = base64Size + 1;
	img_src_buffer = malloc(img_src_buffer_len);
	if (img_src_buffer == NULL) {
		ESP_LOGE(TAG, "malloc fail. img_src_buffer_len %d", img_src_buffer_len);
	} else {
		esp_err_t ret = Image2Base64(localFileName, st.st_size, img_src_buffer, img_src_buffer_len);
		ESP_LOGI(TAG, "Image2Base64=%d", ret);
		if (ret != 0) {
			ESP_LOGE(TAG, "Error in mbedtls encode! ret = -0x%x", -ret);
		} else {
			// <img src="data:image/jpeg;base64,ENCORDED_DATA" />
			httpd_resp_sendstr_chunk(req, "<img src=\"data:image/jpeg;base64,");
			httpd_resp_send_chunk(req, (char *)img_src_buffer, base64Size);
			httpd_resp_sendstr_chunk(req, "\" />");
		}
	}
	if (img_src_buffer != NULL) free(img_src_buffer);

	/* Finish the file list table */
	httpd_resp_sendstr_chunk(req, "</tbody></table>");

	/* Send remaining chunk of HTML file to complete it */
	httpd_resp_sendstr_chunk(req, "</body></html>");

	/* Send empty chunk to signal HTTP response completion */
	httpd_resp_sendstr_chunk(req, NULL);
	return ESP_OK;
}

static const httpd_uri_t root = {
	.uri	   = "/",
	.method    = HTTP_GET,
	.handler   = root_get_handler,
	/* Let's pass response string in user
	 * context to demonstrate it's usage */
	.user_ctx  = "Hello World!"
};


static httpd_handle_t start_webserver(int port)
{
	// Generate default configuration
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Empty handle to http_server
	httpd_handle_t server = NULL;

	// Purge “Least Recently Used” connection
	config.lru_purge_enable = true;

	// TCP Port number for receiving and transmitting HTTP traffic
	config.server_port = port;

	// Start the httpd server
	//ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	if (httpd_start(&server, &config) == ESP_OK) {
		// Set URI handlers
		ESP_LOGI(TAG, "Registering URI handlers");
		httpd_register_uri_handler(server, &root);
		return server;
	}

	ESP_LOGE(TAG, "Error starting server!");
	return NULL;
}



void http_task(void *pvParameters)
{
	char *task_parameter = (char *)pvParameters;
	ESP_LOGI(TAG, "Start task_parameter=%s", task_parameter);
	char url[64];
	int port = 8080;
	sprintf(url, "http://%s:%d", task_parameter, port);
	ESP_LOGI(TAG, "Starting HTTP server on %s", url);
	//static httpd_handle_t server = NULL;
	//server = start_webserver(port);
	start_webserver(port);

	HTTP_t httpBuf;
	while(1) {
		//Waiting for HTTP event.
		if (xQueueReceive(xQueueHttp, &httpBuf, portMAX_DELAY) == pdTRUE) {
			ESP_LOGI(TAG, "httpBuf.localFileName=[%s]", httpBuf.localFileName);
			localFileName = httpBuf.localFileName;
			ESP_LOGW(TAG, "Open this in your browser %s", url);
		}
	}

	// Never reach here
	ESP_LOGI(TAG, "finish");
	vTaskDelete(NULL);
}
