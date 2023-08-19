#include "sntp_time.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"


static const char *TAG = "SNTP Module";

RTC_DATA_ATTR static int boot_count = 0;
extern bool time_sinc_ok;
time_t device_start_time = 0;
struct tm on_time = {0}; // Hora de encendido
struct tm off_time = {0}; // Hora de apagado
extern char formatted_on_time[6];
extern char formatted_off_time[6];

#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM

void sntp_sync_time(struct timeval *tv)
{
   settimeofday(tv, NULL);
   ESP_LOGI(TAG, "Time is synchronized from custom code");
   sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}
#endif

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
    time_sinc_ok = true;
}

void obtain_time(void)
{
    ESP_LOGI(TAG, "Obtaining time...");
    time_t now = 0;
    struct tm timeinfo = { 0 };
    // Obtiene la hora actual
    time(&now);
    localtime_r(&now, &timeinfo);
    // Formatea la hora en el formato deseado (YYYY-MM-DD HH:MM:SS)
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "Obtained time: %s", formatted_time);
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
    // Espera hasta que se complete la sincronización de tiempo
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "Time synchronized");
}

void update_device_start_time(void) {
    time(&device_start_time);
}

void get_device_uptime(char *uptime_str, size_t max_size) {
    time_t current_time;
    time(&current_time);
    time_t uptime_seconds = current_time - device_start_time;
    int hours=uptime_seconds/3600;
    int minutes=(uptime_seconds%3600)/60;
    int seconds=uptime_seconds%60;
    snprintf(uptime_str, max_size, "%02d:%02d:%02d", hours, minutes, seconds);
}

void power_on_device(void) {
    update_device_start_time();
    ESP_LOGI(TAG, "Device powered on at: %s", ctime(&device_start_time));
}

void set_on_off_times(void) {
    // Establecer la hora de encendido
    on_time.tm_hour = 20;
    on_time.tm_min = 0;
    on_time.tm_sec = 0;

    // Establecer la hora de apagado
    off_time.tm_hour = 8;
    off_time.tm_min = 0;
    off_time.tm_sec = 0;
    
    snprintf(formatted_on_time, sizeof(formatted_on_time), "%02d:%02d", on_time.tm_hour, on_time.tm_min);
    snprintf(formatted_off_time, sizeof(formatted_off_time), "%02d:%02d", off_time.tm_hour, off_time.tm_min);
}